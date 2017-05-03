# """"""""""""""""""""""""""""""""""""""""""""""
# The University of Sheffield
# WYSIWYD Project
#
# SAMpy class for various methods related to processing data
#
# Created on 26 May 2015
#
# @authors: Andreas Damianou, Daniel Camilleri
#
# """"""""""""""""""""""""""""""""""""""""""""""
import matplotlib
matplotlib.use("TkAgg")
import numpy as np
from ConfigParser import SafeConfigParser
import pickle
import matplotlib.mlab as mlab
import matplotlib.pyplot as plt
from SAM.SAM_Core import samOptimiser
from os import listdir
from os.path import join, isdir
import threading
import logging

np.set_printoptions(precision=2)

## \defgroup icubclient_SAM_utils SAM Utils
## @{Utility class for functions required by both interactionSAMModel.py and trainSAMModel.py
## \ingroup icubclient_SAM_source


def initialiseModels(argv, update, initMode='training'):
    """Initialise SAM Model data structure, training parameters and user parameters.

        This method starts by initialising the required Driver from the driver name in argv[3] if it exists
        in SAM_Drivers folder. The standard model parameters and the specific user parameters are then initialised
        and the data is read in by the SAMDriver.readData method to complete the model data structure. This method
        then replicates the model data structure for training with multiple models if it is required in the config
        file loaded by the Driver.

        Args:
            argv_0: dataPath containing the data that is to be trained on.
            argv_1: modelPath containing the path of where the model is to be stored.
            argv_2: driverName containing the name of the driver class that is to be loaded from SAM_Drivers folder.
            update: String having either a value of 'update' or 'new'. 'new' will load the parameters as set in the
                    config file of the driver being loaded present in the dataPath directory. This is used to train a
                     new model from scratch. 'update' will check for an existing model in the modelPath directory and
                     load the parameters from this model if it exists. This is used for retraining a model when new
                     data becomes available.
            initMode: String having either a value of 'training' or 'interaction'. 'training' takes into consideration
                      the value of update in loading the parameters. (Used by trainSAMModel.py) 'interaction' loads
                      the parameters directly from the model if the model exists. (Used by interactionSAMModel.py)

        Returns:
            The output is a list of SAMDriver models. The list is of length 1 when the config file requests a single
            model or a list of length n+1 for a config file requesting multiple models where n is the number of
            requested models. The number of models either depends on the number of directories present in the dataPath
            or from the length of textLabels returned from the SAMDriver.readData method.
    """

    from SAM.SAM_Core import SAMDriver as Driver
    dataPath = argv[0]
    modelPath = argv[1]
    driverName = argv[2]

    logging.info(argv)
    stringCommand = 'from SAM.SAM_Drivers import ' + driverName + ' as Driver'
    logging.info(stringCommand)
    exec stringCommand

    mySAMpy = Driver()
    mode = update
    trainName = dataPath.split('/')[-1]

    # participantList is extracted from number of subdirectories of dataPath
    participantList = [f for f in listdir(dataPath) if isdir(join(dataPath, f))]

    off = 17
    logging.info('-------------------')
    logging.info('Training Settings:')
    logging.info('')
    logging.info('Init mode: '.ljust(off) + str(initMode))
    logging.info('Data Path: '.ljust(off) + str(dataPath))
    logging.info('Model Path: '.ljust(off) + str(modelPath))
    logging.info('Participants: '.ljust(off) + str(participantList))
    logging.info('Model Root Name: '.ljust(off) + str(trainName))
    logging.info('Training Mode:'.ljust(off) + str(mode))
    logging.info('Driver:'.ljust(off) + str(driverName))
    logging.info('-------------------')
    logging.info('Loading Parameters...')
    logging.info('')
    modeConfig = ''
    found = ''
    try:
        parser = SafeConfigParser()
        found = parser.read(dataPath + "/config.ini")

        if parser.has_option(trainName, 'update_mode'):
            modeConfig = parser.get(trainName, 'update_mode')
        else:
            modeConfig = 'update'
        logging.info(modeConfig)
    except IOError:
        pass

    defaultParamsList = ['experiment_number', 'model_type', 'model_num_inducing',
                         'model_num_iterations', 'model_init_iterations', 'verbose',
                         'Quser', 'kernelString', 'ratioData', 'update_mode', 'model_mode',
                         'temporalModelWindowSize', 'optimiseRecall', 'classificationDict',
                         'useMaxDistance', 'calibrateUnknown']

    mySAMpy.experiment_number = None
    mySAMpy.model_type = None
    mySAMpy.kernelString = None
    mySAMpy.fname = None
    mySAMpy.ratioData = None

    if initMode == 'training' and (mode == 'new' or modeConfig == 'new' or 'exp' not in modelPath):
        logging.info('Loading training parameters from:' + str(dataPath) + "/config.ini")
        try:
            default = False
            parser = SafeConfigParser()
            parser.optionxform = str
            found = parser.read(dataPath + "/config.ini")

            mySAMpy.experiment_number = 'exp'

            if parser.has_option(trainName, 'model_type'):
                mySAMpy.model_type = parser.get(trainName, 'model_type')
            else:
                default = True
                mySAMpy.model_type = 'mrd'

            if parser.has_option(trainName, 'model_num_inducing'):
                mySAMpy.model_num_inducing = int(parser.get(trainName, 'model_num_inducing'))
            else:
                default = True
                mySAMpy.model_num_inducing = 30

            if parser.has_option(trainName, 'model_num_iterations'):
                mySAMpy.model_num_iterations = int(parser.get(trainName, 'model_num_iterations'))
            else:
                default = True
                mySAMpy.model_num_iterations = 700

            if parser.has_option(trainName, 'model_init_iterations'):
                mySAMpy.model_init_iterations = int(parser.get(trainName, 'model_init_iterations'))
            else:
                default = True
                mySAMpy.model_init_iterations = 2000

            if parser.has_option(trainName, 'verbose'):
                mySAMpy.verbose = parser.get(trainName, 'verbose') == 'True'
            else:
                default = True
                mySAMpy.verbose = False

            if parser.has_option(trainName, 'optimiseRecall'):
                mySAMpy.optimiseRecall = int(parser.get(trainName, 'optimiseRecall'))
            else:
                default = True
                mySAMpy.optimiseRecall = 200

            if parser.has_option(trainName, 'useMaxDistance'):
                mySAMpy.useMaxDistance = parser.get(trainName, 'useMaxDistance') == 'True'
            else:
                mySAMpy.useMaxDistance = False

            if parser.has_option(trainName, 'calibrateUnknown'):
                mySAMpy.calibrateUnknown = parser.get(trainName, 'calibrateUnknown') == 'True'
            else:
                mySAMpy.calibrateUnknown = False

            if parser.has_option(trainName, 'model_mode'):
                mySAMpy.model_mode = parser.get(trainName, 'model_mode')
                if mySAMpy.model_mode == 'temporal' and parser.has_option(trainName, 'temporalModelWindowSize'):
                        mySAMpy.temporalWindowSize = int(parser.get(trainName, 'temporalModelWindowSize'))
                else:
                    temporalFlag = True
            else:
                default = True
                mySAMpy.model_mode = 'single'

            if parser.has_option(trainName, 'Quser'):
                mySAMpy.Quser = int(parser.get(trainName, 'Quser'))
            else:
                default = True
                mySAMpy.Quser = 2

            if parser.has_option(trainName, 'kernelString'):
                mySAMpy.kernelString = parser.get(trainName, 'kernelString')
            else:
                default = True
                mySAMpy.kernelString = "GPy.kern.RBF(Q, ARD=False) + GPy.kern.Bias(Q) + GPy.kern.White(Q)"

            if parser.has_option(trainName, 'ratioData'):
                mySAMpy.ratioData = int(parser.get(trainName, 'ratioData'))
            else:
                default = True
                mySAMpy.ratioData = 50

            if default:
                logging.info('Default settings applied')

            mySAMpy.paramsDict = dict()
            mySAMpy.loadParameters(parser, trainName)

        except IOError:
            logging.warning('IO Exception reading ', found)
            pass
    else:
        logging.info('Loading parameters from: \n \t' + str(modelPath))
        try:
            parser = SafeConfigParser()
            parser.optionxform = str
            found = parser.read(dataPath + "/config.ini")

            # load parameters from config file
            mySAMpy.experiment_number = modelPath.split('__')[-1]

            modelPickle = pickle.load(open(modelPath+'.pickle', 'rb'))
            mySAMpy.paramsDict = dict()
            for j in parser.options(trainName):
                if j not in defaultParamsList:
                    logging.info(str(j))
                    mySAMpy.paramsDict[j] = modelPickle[j]

            mySAMpy.ratioData = modelPickle['ratioData']
            mySAMpy.model_type = modelPickle['model_type']
            mySAMpy.model_mode = modelPickle['model_mode']
            if mySAMpy.model_mode == 'temporal':
                mySAMpy.temporalModelWindowSize = modelPickle['temporalModelWindowSize']
                mySAMpy.model_type = 'mrd'
            mySAMpy.model_num_inducing = modelPickle['model_num_inducing']
            mySAMpy.model_num_iterations = modelPickle['model_num_iterations']
            mySAMpy.model_init_iterations = modelPickle['model_init_iterations']
            mySAMpy.verbose = modelPickle['verbose']
            mySAMpy.Quser = modelPickle['Quser']
            mySAMpy.optimiseRecall = modelPickle['optimiseRecall']
            mySAMpy.kernelString = modelPickle['kernelString']
            mySAMpy.calibrated = modelPickle['calibrated']

            # try loading classification parameters for multiple model implementation
            try:
                mySAMpy.useMaxDistance = modelPickle['useMaxDistance']
            except:
                logging.warning('Failed to load useMaxDistace. Possible reasons: '
                                'Not saved or multiple model implementation')
            mySAMpy.calibrateUnknown = modelPickle['calibrateUnknown']
            if mySAMpy.calibrateUnknown:
                mySAMpy.classificationDict = modelPickle['classificationDict']

        except IOError:
            logging.warning('IO Exception reading ', found)
            pass

    if 'exp' in modelPath or 'best' in modelPath or 'backup' in modelPath:
         fnameProto = '/'.join(modelPath.split('/')[:-1]) + '/' + dataPath.split('/')[-1] + '__' + driverName + \
                             '__' + mySAMpy.model_type + '__' + str(mySAMpy.experiment_number)
    else:
        fnameProto = modelPath + dataPath.split('/')[-1] + '__' + driverName + '__' + mySAMpy.model_type + \
                             '__' + str(mySAMpy.experiment_number)

    logging.info('Full model name: ' + str(fnameProto))
    logging.info('-------------------')
    logging.info('')

    mySAMpy.save_model = False
    mySAMpy.economy_save = True
    mySAMpy.visualise_output = False
    # test_mode = True

    mySAMpy.readData(dataPath, participantList)

    if mySAMpy.model_mode != 'temporal':
        # get list of labels
        mySAMpy.textLabels = list(set(mySAMpy.L))

        # convert L from list of strings to array of indices
        mySAMpy.L = np.asarray([mySAMpy.textLabels.index(i) for i in mySAMpy.L])[:, None]
        mySAMpy.textLabels = mySAMpy.textLabels
    else:
        mySAMpy.X, mySAMpy.Y = transformTimeSeriesToSeq(mySAMpy.Y1, mySAMpy.temporalModelWindowSize)
        mySAMpy.L, mySAMpy.tmp = transformTimeSeriesToSeq(mySAMpy.U1, mySAMpy.temporalModelWindowSize)

    mm = [mySAMpy]
    # mm.append(mySAMpy)
    # mm[0] contains root model
    # this is the only model in the case of a single model
    # or contains all info for the rest of the models in case of multiple models
    #

    if mySAMpy.model_mode == 'single' or mySAMpy.model_mode == 'temporal':
        mm[0].participantList = ['all']
    else:
        mm[0].participantList = ['root'] + mySAMpy.textLabels

    for k in range(len(mm[0].participantList)):
        if mm[0].participantList[k] == 'all':
            normaliseData = True
            minData = len(mm[k].L)
            mm[0].fname = fnameProto
            mm[0].model_type = mySAMpy.model_type
            Ntr = int(mySAMpy.ratioData * minData / 100)
        else:
            if k > 0:
                mm.append(Driver())
                # extract subset of data corresponding to this model
                inds = [i for i in range(len(mm[0].Y['L'])) if mm[0].Y['L'][i] == k - 1]
                mm[k].Y = mm[0].Y['Y'][inds]
                mm[k].L = mm[0].Y['L'][inds]
                mm[k].Quser = mm[0].Quser
                mm[k].verbose = mm[0].verbose
                logging.info('Object class: ' + str(mm[0].participantList[k]))
                minData = len(inds)
                mm[k].fname = fnameProto + '__L' + str(k - 1)
                mm[0].listOfModels.append(mm[k].fname)
                mm[k].model_type = 'bgplvm'
                Ntr = int(mySAMpy.ratioData * minData / 100)
                normaliseData = True
            else:
                normaliseData = False
                mm[0].listOfModels = []
                mm[0].fname = fnameProto
                mm[0].SAMObject.kernelString = ''
                minData = len(mm[0].L)
                Ntr = int(mySAMpy.ratioData * minData / 100)
            mm[k].modelLabel = mm[0].participantList[k]

        if mm[0].model_mode != 'temporal':

            [Yall, Lall, YtestAll, LtestAll] = mm[k].prepareData(mm[k].model_type, Ntr,
                                                                 randSeed=0,
                                                                 normalise=normaliseData)
            mm[k].Yall = Yall
            mm[k].Lall = Lall
            mm[k].YtestAll = YtestAll
            mm[k].LtestAll = LtestAll
        elif mm[0].model_mode == 'temporal':
            [Xall, Yall, Lall, XtestAll, YtestAll, LtestAll] = mm[k].prepareData(mm[k].model_type, Ntr,
                                                                                 randSeed=0,
                                                                                 normalise=normaliseData)
            mm[k].Xall = Xall
            mm[k].Yall = Yall
            mm[k].Lall = Lall
            mm[k].XtestAll = XtestAll
            mm[k].YtestAll = YtestAll
            mm[k].LtestAll = LtestAll

        logging.info('minData = ' + str(minData))
        logging.info('ratioData = ' + str(mySAMpy.ratioData))
    logging.info('-------------------------------------------------------------------------------------------------')
    if initMode == 'training':
        samOptimiser.deleteModel(modelPath, 'exp')
        for k in range(len(mm[0].participantList)):
            # for k = 0 check if multiple model or not
            if mm[0].participantList[k] != 'root':

                logging.info("Training with " + str(mm[0].model_num_inducing) + ' inducing points for ' +
                             str(mm[0].model_init_iterations) + '|' + str(mm[0].model_num_iterations))
                logging.info("Fname:" + str(mm[k].fname))

                mm[k].training(mm[0].model_num_inducing, mm[0].model_num_iterations,
                               mm[0].model_init_iterations, mm[k].fname, mm[0].save_model,
                               mm[0].economy_save, keepIfPresent=False, kernelStr=mm[0].kernelString)

                if mm[0].visualise_output:
                    ax = mm[k].SAMObject.visualise()
                    visualiseInfo = dict()
                    visualiseInfo['ax'] = ax
                else:
                    visualiseInfo = None
    else:
        for k in range(len(mm[0].participantList)):
            # for k = 0 check if multiple model or not
            if mm[0].participantList[k] != 'root':
                logging.info("Training with " + str(mm[0].model_num_inducing) + ' inducing points for ' +
                             str(mm[0].model_init_iterations) + '|' + str(mm[0].model_num_iterations))

                mm[k].training(mm[0].model_num_inducing, mm[0].model_num_iterations,
                               mm[0].model_init_iterations, mm[k].fname, mm[0].save_model,
                               mm[0].economy_save, keepIfPresent=True, kernelStr=mm[0].kernelString)

    return mm


def varianceClass(varianceDirection, x, thresh):
    """
        Utility function to perform threshold or range checking.

        Args:
            varianceDirection : List of strings with the conditions to check.
            x : The value to be checked.
            thresh : The threshold against which x is to be checked given the checks in varianceDirection.

        Returns:
            Boolean True or False confirming arg x validates varianceDirection conditions for the threshold.
    """
    if varianceDirection == ['greater', 'smaller']:
        return thresh[0] < x < thresh[1]
    elif varianceDirection == ['smaller', 'greater']:
        return thresh[0] > x > thresh[1]
    elif varianceDirection == ['greater']:
        return x > thresh
    elif varianceDirection == ['smaller']:
        return x < thresh


class TimeoutError(Exception):
    """Custom TimeoutError Exception.

        Description:
            Class used to raise TimeoutError Exceptions.
    """
    pass


class InterruptableThread(threading.Thread):
    """Class to launch a function inside of a separate thread.
    """
    def __init__(self, func, *args, **kwargs):
        """
            Initialise the interruptible thread.
        """
        threading.Thread.__init__(self)
        self._func = func
        self._args = args
        self._kwargs = kwargs
        self._result = None

    def run(self):
        """
            Run the function.
        """
        self._result = self._func(*self._args, **self._kwargs)

    @property
    def result(self):
        """
        Returns:
            Result of the function.
        """
        return self._result


class timeout(object):
    """
        Class to terminate a function running inside of a separate thread.
    """
    def __init__(self, sec):
        """
            Initialise the timeout function.
        """
        self._sec = sec

    def __call__(self, f):
        """
            Initialise an interruptible thread and start the thread.
        """
        def wrapped_f(*args, **kwargs):
            it = InterruptableThread(f, *args, **kwargs)
            it.start()
            it.join(self._sec)
            if not it.is_alive():
                return it.result
            raise TimeoutError('execution expired')
        return wrapped_f


def plotKnownAndUnknown(varDict, colour, axlist, width=[0.2, 0.2], factor=[(0, 0.6), (0.4, 1)], plotRange=False):
    """
        Utility function to plot variances of known and unknown as gaussian distributions.

        Args:
            varDict : Dictionary containing the mean and variances of known and unknown for different sections of data.
            colour : List of strings with the colours to be used for each plot.
            axlist : Plot object to pass in and update.
            width : List of floats with the linewidth for the plots.
            factor : List of tuples with factors for the plotting of ranges.
            plotRange : Boolean to plot a range together with gaussian distributions or not.

        Returns:
            Plot object for the generated plot.
    """
    count = 0
    for k, j in enumerate(varDict.keys()):
        if len(varDict[j]) > 0 and 'Results' not in j:
            [mlist, vlist, rlist] = meanVar_varianceDistribution(varDict[j])
            axlist = plotGaussFromList(mlist, vlist, rlist, colour[count], j, width[count], factor[count], axlist, plotRange)
            count += 1

    return axlist


def bhattacharyya_distance(mu1, mu2, var1, var2):
    """
        Calculates a measure for the separability of two univariate gaussians.

        Returns the bhattacharyya distance that is used to optimise for separability between known and unknown classes when these are modelled as univariate gaussians.

        Args:
            mu1: Float with mean of distribution 1.
            mu2: Float with mean of distribution 2.
            var1: Float with variance of distribution 1.
            var2: Float with variance of distribution 2.

        Returns:
            Returns a float with the bhattacharyya distance between the two distributions.
    """

    t1 = float(var1/var2) + float(var2/var1) + 2
    t2 = np.log(0.25*t1)
    t3 = float(mu1-mu2)*float(mu1-mu2)
    t4 = t3/float(var1+var2)
    return 0.25*t2 + 0.25*t4


def plot_confusion_matrix(cm, targetNames, title='Confusion matrix', cmap=plt.cm.inferno):
    """Generate and display a confusion matrix.

        This method plots a formatted confusion matrix from the provided array and target names.

        Args:
            cm: Square numpy array containing the values for the confusion matrix.
            targetNames: labels for the different classes.
            title: Title of the plot.
            cmap: Matplotlib colourmap for the plot.

        Returns:
            No return. Blocking call to matplotlib.plot.
    """
    plt.figure()
    plt.imshow(cm, interpolation='nearest', cmap=cmap)
    plt.title(title)
    plt.colorbar()
    tick_marks = np.arange(len(targetNames))
    plt.xticks(tick_marks, targetNames, rotation=45)
    plt.yticks(tick_marks, targetNames)
    plt.tight_layout()
    plt.ylabel('True label')
    plt.xlabel('Predicted label')
    plt.show(block=True)


def plotGaussFromList(mlist, vlist, rlist, colour, label, width, factor, axlist, plotRange=False):
    """
    Plot multiple Gaussians from a list on the same plot.

    Args:
        mlist: List of float means.
        vlist: List of float variances.
        rlist: List of float data ranges.
        colour: Colour for the plots.
        label: Label for the plot.
        width: Width of line in the plot.
        factor: Factor for the height of the ranges to make them more aesthetic.
        axlist: List of axes.
        plotRange: Boolean to plot ranges or not.

    Returns:
        List of axes.
    """
    numPlots = len(mlist)

    if len(axlist) == 0:
        # f, axlist = plt.subplots(1, numPlots, figsize=(24.0, 15.0))
        f, axlist = plt.subplots(1, numPlots, figsize=(12.0, 7.5))
        for k, j in enumerate(axlist):
            if k < numPlots - 2:
                j.set_title('D ' + str(k), fontsize=20)
            elif k == numPlots - 2:
                j.set_title('Sum', fontsize=20)
            elif k > numPlots - 2:
                j.set_title('Mean', fontsize=20)
            j.set_xticks([])
            j.set_yticks([])

    for j in range(numPlots):
        sigma = np.sqrt(vlist[j])
        rangeData = rlist[j][1] - rlist[j][0]
        x = np.linspace(rlist[j][0] - (rangeData / 2), rlist[j][1] + (rangeData / 2), 100)
        y = mlab.normpdf(x, mlist[j], sigma)
        axlist[j].plot(x, y, colour, label=label)
        if plotRange:
            axlist[j].plot([rlist[j][1], rlist[j][1]], [max(y)*factor[0], max(y)*factor[1]], '--'+colour, linewidth=width)
            axlist[j].plot([rlist[j][0], rlist[j][0]], [max(y)*factor[0], max(y)*factor[1]], '--'+colour, linewidth=width)

    return axlist


def solve_intersections(m1, m2, std1, std2):
    """
    Solve for the intersection/s of two Gaussian distributions.

    Args:
        m1: Float Mean of Gaussian 1.
        m2: Float Mean of Gaussian 2.
        std1: Float Standard Deviation of Gaussian 1.
        std2: Float Standard Deviation of Gaussian 2.

    Returns:
        Points of intersection for the two Gaussian distributions.
    """
    a = 1/(2*std1**2) - 1/(2*std2**2)
    b = m2/(std2**2) - m1/(std1**2)
    c = m1**2 / (2*std1**2) - m2**2 / (2*std2**2) - np.log(std2/std1)
    return np.roots([a, b, c])


def PfromHist(sample, hist, binWidth):
    """
    Calulates the probability of a sample from a histogram.

    Args:
        sample : Float with sample to be tested.
        hist : Numpy array with normalised histogram probability values.
        binWidth : Float indicating the width for each probability bin.

    Returns:
        Probability ranging from 0 to 1 for sample with respect to the histogram.
    """
    idx = np.asarray(sample)//binWidth
    idx = idx.astype(np.int)
    pList = []
    for j in range(len(idx)):
        pList.append(hist[j][idx[j]])
    return pList


def meanVar_varianceDistribution(dataList):
    """
    Calculate list of means, variances and ranges for the data in the dataList.

    Args:
        dataList: List of numpy arrays containing the data to check.

    Returns:
        List of means, list of variances and list of ranges. One for each array in the dataList.
    """
    mlist = []
    vlist = []
    rlist = []

    dataArray = np.asarray(dataList)
    if len(dataArray.shape) == 1:
        dataArray = dataArray[:, None]

    numPlots = dataArray.shape[1]

    for j in range(numPlots):
        # if j < numPlots:
        h = dataArray[:, j]
        # elif j == numPlots:
        #     h = np.sum(dataArray, 1)
        # elif j == numPlots + 1:
        #     h = np.mean(dataArray, 1)

        mean = np.mean(h)
        variance = np.var(h)
        rlist.append((min(h), max(h)))
        mlist.append(mean)
        vlist.append(variance)

    return mlist, vlist, rlist


def bhattacharyya_dict(m, v):
    """
    Calculate bhattacharyya distances for each item in the dictionaries.

    Args:
        m : Dictionary of means.
        v : Dictionary of variances.

    Returns:
        List of bhattacharyya distances for the input dictionaries.
    """
    knownLabel = None
    unknownLabel = None
    dists = []

    for j in m.keys():
        if 'known' == j.lower().split(' ')[1]:
            knownLabel = j
        elif 'unknown' in j.lower().split(' ')[1]:
            unknownLabel = j

    if unknownLabel is not None and knownLabel is not None:
        numDists = len(m[knownLabel])
        for j in range(numDists):
            dists.append(
                bhattacharyya_distance(m[knownLabel][j], m[unknownLabel][j], v[knownLabel][j], v[unknownLabel][j]))
        return dists
    else:
        return None


def smooth1D(x, window_len=11, window='hanning'):
    """Smooth the data using a window with a requested size.

    This method is based on the convolution of a scaled window with the signal. The signal is prepared by introducing reflected copies of the signal (with the window size) in both ends so that transient parts are minimized in the beginning and end part of the output signal.

    input:
        x: The input signal.
        window_len: The dimension of the smoothing window; should be an odd integer.
        window: The type of window from 'flat', 'hanning', 'hamming', 'bartlett', 'blackman' flat window will produce a moving average smoothing.

    output:
        The smoothed signal.
    """

    if x.ndim != 1:
        raise ValueError("smooth only accepts 1 dimension arrays.")

    if x.size < window_len:
        raise ValueError("Input vector needs to be bigger than window size.")

    if window_len < 3:
        return x

    if window not in ['flat', 'hanning', 'hamming', 'bartlett', 'blackman']:
        raise ValueError("Window is on of 'flat', 'hanning', 'hamming', 'bartlett', 'blackman'")

    s = np.r_[x[window_len-1:0:-1], x, x[-1:-window_len:-1]]
    # logging.info((len(s))
    if window == 'flat':  # moving average
        w = np.ones(window_len, 'd')
    else:
        w = eval('np.'+window+'(window_len)')

    y = np.convolve(w/w.sum(), s, mode='valid')
    off = 0
    if window_len % 2 > 0:
        off = 1

    return y[(window_len/2-1):-((window_len/2)+off)]


def transformTimeSeriesToSeq(Y, timeWindow, offset=1, normalised=False, reduced=False, noY=False, doOffset=False):
    # TODO add parameter for number of points to skip between sampled windows
    """
    Utility function to convert a time series into multiple time windows with additional functionality.

    Args:
        Y : Time series data.
        timeWindow : Length of the time window.
        offset : Number of non-overlapping frames between successive time windows.
        normalised : Boolean to normalise time windows with respect to the starting frame.
        reduced : Boolean to remove the starting frame if normalisation is enabled since this frame contains 0.
        noY : Boolean to return time windows of labels together.
        doOffset : In future versions this parameter will enable skipping data points between sampled windows.

    Returns:
        X : numpy array of size (numberWindows x lengthOfWindow) containing the time series split up into windows.
        Y : numpy array of size (numberWindows x lengthOfWindow) containing the labels for each frame in the time series split into windows.
    """
    Ntr, D = Y.shape
    if noY:
        blocksNumber = (Ntr - timeWindow + 1) // offset
    else:
        blocksNumber = (Ntr - timeWindow) // offset

    if normalised and reduced:
        X = np.zeros((blocksNumber, (timeWindow - 1) * D))
    else:
        X = np.zeros((blocksNumber, timeWindow * D))

    if not noY:
        Ynew = np.zeros((blocksNumber, D))
    else:
        Ynew = None

    for i in range(blocksNumber):
        base = i * offset
        tmp = Y[base:base + timeWindow, :].T

        if normalised:
            tmp = np.subtract(tmp, tmp[:, 0][:, None])
            if reduced:
                tmp = np.delete(tmp, 0, 1)
        X[i, :] = tmp.flatten().T

        if not noY:
            Ynew[i, :] = Y[base + timeWindow, :]
    return X, Ynew

## @}
