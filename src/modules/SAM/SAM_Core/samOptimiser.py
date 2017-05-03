#!/usr/bin/env python
# """"""""""""""""""""""""""""""""""""""""""""""
# The University of Sheffield
# WYSIWYD Project
#
# A class that implements Bayesian Optimisation of SAM model parameters
#
# Created on 20 July 2016
#
# @author: Daniel Camilleri
#
# """"""""""""""""""""""""""""""""""""""""""""""

import numpy as np
import time
import os
import subprocess
import pickle
import sys
import glob
import shutil
import copy
from ConfigParser import SafeConfigParser
np.set_printoptions(precision=2)

optNotFound = False
try:
    import GPyOpt
except:
    print 'GPyOpt not found'
    optNotFound = True
    pass

## @ingroup icubclient_SAM_Core
class modelOptClass(object):
    """
        Class to perform optimisation of SAM Models.

        This class reads in the parameters for optimisation from the `[Optimisation]` section found in the config.ini present in the data directory passed in as dataDir. Parameters are set for variables that are present in the `[driver_name]` section of the same config.ini in the following manner `variableName = [parameterType:commaSeparatedParameterValues]`.\n

        Parameters can be : \n
        1) __discreteInt__ : Equally spaced integers in the form of `x = [discreteInt:start,interval,end]`. \n
        2) __discreteFloat__ : Equally spaced floats in the form of `x = [discreteFloat:start,interval,end]`. \n
        3) __continuous__ : Continuous range in the form of `x = [continuous:start,end]`. \n
        4) __list__ : Use one parameter value at a time from the comma separated list of possible parameter values in the form of `x = [list:a,b,c,d]` where x is set to just one of the values in the list. \n
        5) __bool__ : In the form of `x = [bool]` where x = 1 or 0. Similar to `x = [list:1,0]`. \n
        6) __combination__ : Use multiple parameters at a time from the comma separated list of possible parameters values in the form of `x = [combination:a,b,c,d]` where x is set to a combination of parameters. Length of the values list in x ranges from 1 to number of comma separated parameter values. \n

        Examples:
            Example config.ini

            [model_options]
            driver = driverName # driver present in SAM_Drivers folder
            modelNameBase = modelName # user set model name

            [modelName]
            # default parameters for the training section which must always be present.
            update_mode = `new` # `new` or `false`. `New` fill train a new model with the following parameters and delete the old one. `False` will check for the availbility of an already trained model and if one is available load it together with its parameters ignoring the rest of this file.
            experiment_number = 0 # experiment number in case different models are required to be compared
            model_type = mrd # model type can be `mrd` or `bgplvm`
            model_mode = single # model_mode can be `single` or `multiple`
            model_num_inducing = 170 # any integer number. Generally < 200 for performance considerations
            model_num_iterations = 50 # any integer number. High numbers mean increased training time
            model_init_iterations = 450 # any integer number. High numbers mean increased training time
            verbose = True # `True` or `False` will turn logging to stdout on or off. Logging to file is always on.
            Quser = 10 # Number of target dimensions for the output latent space. Higher numbers mean a more detailed latent space but very sparse clusters. Higher dimensionality also requires more input data for good generalisation.
            ratioData = 80 # Train/Test split. 80% train, 20% test
            kernelString = "GPy.kern.RBF(Q, ARD=False) + GPy.kern.Bias(Q) + GPy.kern.White(Q)" # This is the kernel used by the Gaussian Process. Keep this constant. Future releases will make this a changeable parameter
            optimiseRecall = 0 # This parameter sets the number of optimisations that occur during recall. If 0 no optimisations are made and recall becames similair to nearest neighbour analysis.
            calibrateUnknown = True # This parameter triggers the learning of known/unknown classification.
            # start of unique model parameters. These parameters are usually parameters to modify and alter the signal processing that occurs in user defined readData function. The following are examples.
            thresholdMovement = 1
            components = pos,vel
            joints = head
            windowOffsetPercent = 12.5
            thresholdPercent = 22
            moveThresh = 0.01

            [Optimisation]
            # default parameters for the optimisation section which must always be present.
            acquisitionFunction = 'EI' # Can be either 'MPI' : maximum probability of improvement, 'EI'  : Expected Improvement or 'UCB' : Upper class bound
            # custom optimisation parameters which must be a subset of the default and custom parameters in the previous section
            model_num_inducing = [discreteInt:20,50,220]
            thresholdMovement = [bool]
            components = [combination:pos,vel,acc]
            joints = [list:head,chest,right hand,left hand,right arm,left arm]
            windowOffsetPercent = [discreteFloat:10.0,1.5,22.5]
            thresholdPercent = [continuous:13.5,]
    """
    def __init__(self, fName, dataDir, modelDir, driverName, mode, baseName, persistence, windowed, verbose):
        """
        Initialisation for modelOptClass

        Args:
            fName: File name of the model to use as a starting point for the optimisation
            dataDir: Directory containing the data to be trained
            modelDir: Directory where to save optimised model and temporary models created during optimisation
            driverName: Name of driver to use for training which must be present in SAM_Drivers folder
            mode: This is a legacy parameter that changed the behaviour of saving models. This parameter is not in use anymore.
            baseName: Base name of model to be trained. Model filenames take the form of __baseName_driverName_modelType_modelVersion.pickle__ with a corresponding __baseName_driverName_modelType_modelVersion_model.h5__. modelVersion can be `best`, `exp<experiment number>`, or `backup`.
            persistence	: `'True'` or `'False'` indicating terminals opened by samOptimiser stay open even after process termination
            windowed 	: `'True'` or `'False'` indicating separate terminals will be opened by samOptimiser for each training and interaction process
            verbose		: `'True'` or `'False'` switching on or off logging to stdout

        Returns:
            None
        """
        try:
            import GPyOpt
            self.fName = fName
            self.dataDir = dataDir
            self.modelDir = modelDir
            self.driverName = driverName
            self.baseName = baseName
            self.persistence = persistence
            self.verbose = verbose
            self.devnull = None
            self.windowed = windowed
            if not self.windowed:
                self.devnull = open('/dev/null', 'w')
            self.numEvals = 0
            self.penalty = 10000000000000000000
            self.mode = mode
            self.parser = None
            self.sectionBackup = None
            self.sectionOpt = None
            self.modelPresent = False
            self.bestOptions = None
            self.domain = None
            self.bestError = None
            self.currIterSettings = None
            self.acquisitionFunction = None
            self.trainProcess = None
            self.resultsList = []
            self.currFiles = None
            self.configured = self.configOptimisation()
            print self.configured[1]
        except:
            msg = 'Cannot find GPyOpt package. Make sure it is installed and added to PYTHONPATH'
            print msg
            self.configured = [False, msg]

    def configOptimisation(self):
        """
            Configure optimisation parameters from config.ini

            Reads in config.ini parameters and sets up the optimisation landscape from these parameters. Also reads in the performance of a current model if one is available to set that as the performance to beat.

            Returns:
                None
        """
        self.parser = SafeConfigParser()
        self.parser.optionxform = str
        try:
            # check config file exists
            found = self.parser.read(self.dataDir + "/config.ini")
            if found:
                # open and check if Optimisation section is present in config.ini
                if (self.parser.has_section('Optimisation') and self.parser.has_section(
                        self.baseName)):
                    # create backup of current self.driverName section
                    self.sectionBackup = dict(self.parser.items(self.baseName))

                    # load Optimisation section
                    self.sectionOpt = dict(self.parser.items('Optimisation'))
                    if len(self.sectionOpt) == 1 and 'acquisitionFunction' in self.sectionOpt:
                        return [False,
                                'config.ini found, Optimisation and ' + self.baseName +
                                ' sections found but Optimisation section does not contain parameters to optimise']
                    else:
                        # create backup if model in modelDir exists
                        self.modelPresent = self.copyModel('backup', 'normal')

                        # load performance of current model if one is available and track performance
                        self.bestError = 0
                        try:
                            if len(self.currFiles) > 0:
                                for j in self.currFiles:
                                    if '.pickle' in j and '__L' not in j:
                                        modelPickle = pickle.load(open(j, 'rb'))
                                        testConf = modelPickle['overallPerformance']
                                        np.fill_diagonal(testConf, 0)
                                        self.bestError += np.sum(testConf)
                                # after combining all errors into one value copy and rename files to best
                                # which contains the best performing model so far
                                # best model will only be present if correct computation of bestError occurs
                                self.copyModel('best', 'normal')
                                self.bestOptions = copy.deepcopy(self.parser.items(self.baseName))
                            else:
                                print 'No model present'
                                self.bestError = self.penalty
                        except:
                            print 'testConf key not present in .pickle file'
                            self.bestError = self.penalty

                        # iterate over keys of sectionOpt to create domain of optimisation problem
                        self.acquisitionFunction = 'MPI'
                        self.domain = []  # list of dictionaries
                        # armedBanditsMode = True
                        self.numPossibilities = 1
                        for i, v in self.sectionOpt.iteritems():
                            if i == 'acquisitionFunction':
                                # possible acquisition functions
                                # 'MPI' : maximum probability of improvement
                                # 'EI'  : Expected Improvement
                                # 'UCB' : Upper class bound
                                if v == 'MPI' or v == 'EI' or v == 'UCB':
                                    self.acquisitionFunction = v
                            elif i in self.sectionBackup:
                                opts = v.partition('[')[-1].rpartition(']')[0]
                                sects = opts.split(':')
                                tempDict = dict()
                                if sects[0] == 'discreteInt':
                                    lims = sects[1].split(',')
                                    arr = np.arange(int(lims[0]), int(lims[2]), int(lims[1]))
                                    arr = np.hstack((arr, int(lims[2])))
                                    self.numPossibilities *= len(arr)
                                    tempDict['name'] = i
                                    tempDict['type'] = 'discrete'
                                    tempDict['domain'] = arr
                                    tempDict['description'] = sects[0]
                                    self.domain.append(tempDict)
                                elif sects[0] == 'discreteFloat':
                                    lims = sects[1].split(',')
                                    arr = np.arange(float(lims[0]), float(lims[2]), float(lims[1]))
                                    arr = np.hstack((arr, float(lims[2])))
                                    self.numPossibilities *= len(arr)
                                    tempDict['name'] = i
                                    tempDict['type'] = 'discrete'
                                    tempDict['domain'] = arr
                                    tempDict['description'] = sects[0]
                                    self.domain.append(tempDict)
                                elif sects[0] == 'continuous':
                                    # armedBanditsMode = False
                                    lims = sects[1].split(',')
                                    tempDict['name'] = i
                                    tempDict['type'] = 'continuous'
                                    tempDict['domain'] = (float(lims[0]), float(lims[1]))
                                    tempDict['description'] = sects[0]
                                    self.domain.append(tempDict)
                                elif sects[0] == 'bool':
                                    tempDict['name'] = i
                                    tempDict['type'] = 'discrete'
                                    tempDict['domain'] = np.array((0, 1))
                                    tempDict['description'] = sects[0]
                                    self.numPossibilities *= 2
                                    self.domain.append(tempDict)
                                elif sects[0] == 'combination':
                                    splitList = sects[1].split(',')
                                    for b in splitList:
                                        tempDict = dict()
                                        tempDict['name'] = b
                                        tempDict['type'] = 'discrete'
                                        tempDict['domain'] = np.array((0, 1))
                                        self.numPossibilities *= 2
                                        tempDict['description'] = sects[0]
                                        tempDict['groupName'] = i
                                        self.domain.append(tempDict)
                                elif sects[0] == 'list':
                                    splitList = sects[1].split(',')
                                    tempDict = dict()
                                    tempDict['name'] = i
                                    tempDict['type'] = 'discrete'
                                    tempDict['domain'] = np.arange(0, len(splitList), 1)
                                    tempDict['description'] = sects[0]
                                    tempDict['groupName'] = i
                                    tempDict['values'] = splitList
                                    self.numPossibilities *= len(splitList)
                                    self.domain.append(tempDict)
                            else:
                                print 'ignoring ', i

                        # if armedBanditsMode :
                        #     for j in self.domain:
                        #         j['type'] = 'bandit'

                        for j in self.domain:
                            print j
                            print

                        return [True, 'Optimisation configuration ready']
                else:
                    return [False,
                            'config.ini found at ' + self.dataDir + ' but does not contain Optimisation and/or ' +
                            self.driverName + ' section']
            else:
                return [False, 'config.ini not present in ' + self.dataDir]
        except:
            return [False, 'Initialising parameters failed']

    def f(self, x):
        """
        Optimisation Evaluation Function

        This function evaluates the current values chosen by the optimiser in `x` by writing these parameter values to the respective parameters in the training parameters section of the config.ini and subsequently making a system call to trainSAMModel.

        Args:
            x : List of current evaluation parameters chosen by the optimiser.

        Returns:
             Total error for the current training parameters. Error is the weighted sum total of the confusion matrix created during testing of the model as part of training.
        """
        self.numEvals += 1
        print 'Trial ', self.numEvals, 'out of', self.numPossibilities, 'possibilities'
        for j in range(len(x[0])):
            print self.domain[j]['name'], ' : ', x[0][j]
        print
        self.currIterSettings = self.sectionBackup
        combinationDicts = dict()
        for j in range(len(x[0])):
            if self.domain[j]['description'] == 'combination':
                if x[0][j] == 1:
                    val = True
                else:
                    val = False
                if val:
                    if self.domain[j]['groupName'] in combinationDicts:
                        combinationDicts[self.domain[j]['groupName']].append(self.domain[j]['name'])
                    else:
                        combinationDicts[self.domain[j]['groupName']] = [self.domain[j]['name']]
            elif self.domain[j]['description'] == 'discreteInt':
                self.parser.set(self.baseName, self.domain[j]['name'], str(int(x[0][j])))
            elif self.domain[j]['description'] == 'list':
                self.parser.set(self.baseName, self.domain[j]['name'], self.domain[j]['values'][int(x[0][j])])
            elif self.domain[j]['description'] == 'bool':
                if x[0][j] == 1:
                    val = 'True'
                else:
                    val = 'False'
                self.parser.set(self.baseName, self.domain[j]['name'], val)
            else:
                self.parser.set(self.baseName, self.domain[j]['name'], str(x[0][j]))
        for name, val in combinationDicts.iteritems():
            self.parser.set(self.baseName, name, ','.join(val) + ',')

        # for t in self.parser.items(self.baseName):
            # print t

        # print
        # print
        self.parser.write(open(self.dataDir + "/config.ini", 'wb'))

        args = ' '.join([self.dataDir, self.modelDir, self.driverName, 'new', self.baseName])

        cmd = self.fName + ' ' + args
        # NEW
        # cmd = trainPath + ' -- ' + args
        if self.persistence:
            command = "bash -c \"" + cmd + "; exec bash\""
        else:
            command = "bash -c \"" + cmd + "\""

        if self.verbose:
            print 'cmd: ', cmd

        # if self.windowed:
        deleteModel(self.modelDir, 'exp')
        if True:
            self.trainProcess = subprocess.Popen(['xterm', '-e', command], shell=False)
        else:
            self.trainProcess = subprocess.Popen([cmd], shell=True, stdout=self.devnull, stderr=self.devnull)

        ret = None
        cnt = 0
        totalTime = 0
        while ret is None:
            ret = self.trainProcess.poll()
            time.sleep(5)
            cnt += 1
            if cnt > 5:
                totalTime += 1
                print 'Training ...', totalTime * 0.5, 'minutes elapsed'
                cnt = 0

        currError = 0
        print 'poll return:', ret
        # if len(self.currFiles) == 0:
        #     self.modelPresent = self.copyModel('backup', 'normal')
        #     self.copyModel('best', 'normal')
        if ret == 0:
            self.currFiles = [j for j in glob.glob('__'.join(self.modelDir.split('__')[:3]) + '*') if 'exp' in j]
            for j in self.currFiles:
                if '.pickle' in j and '__L' not in j:
                    modelPickle = pickle.load(open(j, 'rb'))
                    testConf = modelPickle['overallPerformance']
                    print 'Confusion Matrix: ', testConf
                    np.fill_diagonal(testConf, 0)
                    # introduce a factor to give favour to specific classifications
                    factorMat = np.ones(testConf.shape)
                    factorMat[:-1, -1] = 0.5
                    print 'factorMat', factorMat
                    print 'testConf', testConf
                    print 'modified', testConf*factorMat
                    currError += np.sum(testConf*factorMat)
                    print
                    print 'Current cumulative error: ', currError
                    if currError < self.bestError:
                        deleteModel(self.modelDir, 'best')
                        self.bestError = currError
                        self.copyModel('best', 'normal')
                        self.bestOptions = copy.deepcopy(self.parser.items(self.baseName))
                        self.parser.write(open(self.dataDir + "/configBest.ini", 'wb'))

        else:
            currError = self.penalty
            print 'Error training model'
            print 'Current cumulative error: ', currError

        print 'Best Error so far : ', self.bestError
        print
        print '-----------------------------------------------------'
        self.resultsList.append([x, currError])
        return currError

    def copyModel(self, newName, direction):
        """
        Utility function to copy model.

        Args:
            newName: Model base name to copy.
            direction: `normal` or `reverse`. `normal` makes a copy of the model with `exp<experiment number>` in the filename to `backup` in the filename. `reverse` makes a copy of `backup` into `exp<experiment number>`.

        Returns:
            True of False indicating success.
        """
        if os.path.isfile(self.modelDir):
            print self.modelDir, ' model file present'
            self.currFiles = [j for j in glob.glob('__'.join(self.modelDir.split('__')[:3]) + '*')
                              if 'backup' not in j and 'best' not in j]
            backupFiles = []
            for k in self.currFiles:
                print 'Original: ', k
                temp = k.split('exp')
                if '__L' in k:
                    temp2 = temp[1].split('__')
                    backupFiles += [temp[0] + newName + '__' + temp2[1]]
                else:
                    temp2 = temp[1].split('.')
                    if 'model' in temp[1]:
                        backupFiles += [temp[0] + newName + '_model.' + temp2[1]]
                    else:
                        backupFiles += [temp[0] + newName + '.' + temp2[1]]
                print 'New:     ', backupFiles[-1]
                print

            if direction == 'reverse':
                for j in range(len(backupFiles)):
                    shutil.copyfile(backupFiles[j], self.currFiles[j])
            else:
                for j in range(len(backupFiles)):
                    shutil.copyfile(self.currFiles[j], backupFiles[j])
            return True
        else:
            print 'No model present'
            self.currFiles = []
            return False


def deleteModel(modelDir, newName):
    """
    Utility function to delete models.

    Args:
        modelDir : Directory containing the model to delete.
        newName : Subset of model files to delete. Either `best`, `backup` or `exp`.

    Returns:
        None

    """
    if os.path.isfile(modelDir):
        print modelDir, ' model file present'
        fileList = [j for j in glob.glob('__'.join(modelDir.split('__')[:3]) + '*') if newName in j]
        for k in fileList:
            os.remove(k)


def main():
    """
        Sets up the modelOptClass and calls GPyOpt optimisation for 200 iterations.

        Same arguments as modelOptClass.init().

        Returns:
            0 if completed successfully. -1 if completed unsuccessfully.
    """
    # Initialisation parameters:
    print optNotFound,  ' ', len(sys.argv)
    if len(sys.argv) >= 9 and not optNotFound:
        a = sys.argv[1]
        b = sys.argv[2]
        c = sys.argv[3]
        d = sys.argv[4]
        e = sys.argv[5]
        f = sys.argv[6]
        per = sys.argv[7] == 'True'
        # per = True
        wind = sys.argv[8] == 'True'
        verb = sys.argv[9] == 'True'

        optModel = modelOptClass(a, b, c, d, e, f, per, wind, verb)
        if optModel.configured[0]:
            myBopt = GPyOpt.methods.BayesianOptimization(f=optModel.f,  # function to optimize
                                                         domain=optModel.domain,  # box-constrains of the problem
                                                         initial_design_numdata=2,  # number data initial design
                                                         acquisition_type=optModel.acquisitionFunction)
            max_iter = 200
            # pickle myBopt to save its initialisation
            # logFilename = os.path.join(b, 'optimiserLog')
            # output = open(logFilename, 'wb')
            # pickle.dump({'optModel', optModel}, output)
            # output.close()

            try:
                myBopt.run_optimization(max_iter)
                optModel.parser.write(open(optModel.dataDir + "/config.ini", 'wb'))
                optModel.copyModel('best', 'reverse')
                return 0
            except:
                # # pickle results list together with optimiser
                # d = pickle.load(open(logFilename, 'r'))
                # d['resultList'] = myBopt.resultsList
                # output = open(logFilename, 'wb')
                # pickle.dump(d, output)
                # output.close()
                os.system("mv " + optModel.dataDir+"/configBest.ini " + optModel.dataDir+"/config.ini")
                # optModel.parser.write(open(optModel.dataDir + "/config.ini", 'wb'))
                return -1
        else:
            return -1
    else:
        print 'GPyOpt package not found or incorrect number of arguments'
        return -1

if __name__ == '__main__':
    main()
