# """"""""""""""""""""""""""""""""""""""""""""""
# The University of Sheffield
# WYSIWYD Project
#
# SAMpy class for implementation of SAM module
#
# Created on 26 May 2015
#
# @authors: Uriel Martinez, Luke Boorman, Andreas Damianou
#
# """"""""""""""""""""""""""""""""""""""""""""""
import sys
import numpy
import os
import cv2
import readline
import yarp
from SAM.SAM_Core import SAMDriver
from SAM.SAM_Core import SAMTesting
import logging

## @ingroup icubclient_SAM_Drivers
class SAMDriver_interaction(SAMDriver):
    """
    Class developed for the implementation of face recognition.
    """
    def __init__(self):
        """
        Initialise class using SAMDriver.__init__ and augment with custom parameters.

        additionalParameterList is a list of extra parameters to preserve between training and interaction.
        """
        SAMDriver.__init__(self)
        self.additionalParametersList = ['imgH', 'imgW', 'imgHNew', 'imgWNew',
                                         'image_suffix', 'pose_index', 'pose_selection']

    def loadParameters(self, parser, trainName):
        """
            Function to load parameters from the model config.ini file.

            Method to load parameters from file loaded in parser from within section trainName and store these parameters in self.paramsDict.

        Args:
            parser: SafeConfigParser with pre-read config file.
            trainName: Section from which parameters are to be read.

        Returns:
            None
        """
        if parser.has_option(trainName, 'imgH'):
            self.paramsDict['imgH'] = int(parser.get(trainName, 'imgH'))
        else:
            self.paramsDict['imgH'] = 400

        if parser.has_option(trainName, 'imgW'):
            self.paramsDict['imgW'] = int(parser.get(trainName, 'imgW'))
        else:
            self.paramsDict['imgW'] = 400

        if parser.has_option(trainName, 'imgHNew'):
            self.paramsDict['imgHNew'] = int(parser.get(trainName, 'imgHNew'))
        else:
            self.paramsDict['imgHNew'] = 200

        if parser.has_option(trainName, 'imgWNew'):
            self.paramsDict['imgWNew'] = int(parser.get(trainName, 'imgWNew'))
        else:
            self.paramsDict['imgWNew'] = 200

        if parser.has_option(trainName, 'image_suffix'):
            self.paramsDict['image_suffix'] = parser.get(trainName, 'image_suffix')
        else:
            self.paramsDict['image_suffix'] = '.ppm'

        if parser.has_option(trainName, 'pose_index'):
            self.paramsDict['pose_index'] = list(parser.get(trainName, 'pose_index').replace('\'', '').split(','))
        else:
            self.paramsDict['pose_index'] = ['']

        if parser.has_option(trainName, 'pose_selection'):
            self.paramsDict['pose_selection'] = int(parser.get(trainName, 'pose_selection'))
        else:
            self.paramsDict['pose_selection'] = 0

    def saveParameters(self):
        """
            Executes SAMDriver.saveParameters to save default parameters.
        """
        SAMDriver.saveParameters(self)


    # """"""""""""""""
    def readData(self, root_data_dir, participant_index, *args, **kw):
        """
        Method which accepts a data directory, reads all the data in and outputs self.Y which is a numpy array with n instances of m length feature vectors and self.L which is a list of text Labels of length n.

        This method reads .ppm images from disk, converts the images to grayscale and serialises the data into a feature vector.

        Args:
            root_data_dir: Data directory.
            participant_index: List of subfolders to consider. Can be left as an empty list.

        Returns:
    """
        if not os.path.exists(root_data_dir):
            logging.error("CANNOT FIND:" + root_data_dir)
        else:
            logging.info("PATH FOUND")

        # Find and build index of available images.......
        data_file_count = numpy.zeros([len(participant_index), len(self.paramsDict['pose_index'])])
        data_file_database = {}
        for count_participant, current_participant in enumerate(participant_index):
            data_file_database_part = {}
            for count_pose, current_pose in enumerate(self.paramsDict['pose_index']):
                current_data_dir = os.path.join(root_data_dir, current_participant + current_pose)
                data_file_database_p = numpy.empty(0, dtype=[('orig_file_id', 'i2'), ('file_id', 'i2'),
                                                             ('img_fname', 'a100')])
                data_image_count = 0
                if os.path.exists(current_data_dir):
                    for fileN in os.listdir(current_data_dir):
                        # parts = re.split("[-,\.]", file)
                        fileName, fileExtension = os.path.splitext(fileN)
                        if fileExtension == self.paramsDict['image_suffix']:  # Check for image file
                            file_ttt = numpy.empty(1, dtype=[('orig_file_id', 'i2'), ('file_id', 'i2'),
                                                             ('img_fname', 'a100')])
                            file_ttt['orig_file_id'][0] = int(fileName)
                            file_ttt['img_fname'][0] = fileN
                            file_ttt['file_id'][0] = data_image_count
                            data_file_database_p = numpy.append(data_file_database_p, file_ttt, axis=0)
                            data_image_count += 1
                    data_file_database_p = numpy.sort(data_file_database_p, order=['orig_file_id'])
                data_file_database_part[self.paramsDict['pose_index'][count_pose]] = data_file_database_p
                data_file_count[count_participant, count_pose] = len(data_file_database_p)
            data_file_database[participant_index[count_participant]] = data_file_database_part

            # To access use both dictionaries data_file_database['Luke']['LR']
            # Cutting indexes to smllest number of available files -> Using file count
        min_no_images = int(numpy.min(data_file_count))

        # Load image data into array......
        # Load first image to get sizes....
        data_image = cv2.imread(
            os.path.join(root_data_dir, participant_index[0] + self.paramsDict['pose_index'][0] + "/" +
                         data_file_database[participant_index[0]][
                             self.paramsDict['pose_index'][0]][0][2]))[:, :, (2, 1, 0)]  # Convert BGR to RGB

        # Data size
        logging.info("Found minimum number of images:" + str(min_no_images))
        logging.info("Image count:" + str(data_file_count))
        logging.info("Found image with dimensions" + str(data_image.shape))
        #    imgplot = plt.imshow(data_image)#[:,:,(2,1,0)]) # convert BGR to RGB

        # Load all images....
        # Data Dimensions:
        # 1. Pixels (e.g. 200x200)
        # 2. Images
        # 3. Person
        # 4. Movement (Static. up/down. left / right)
        set_x = int(data_image.shape[0])
        set_y = int(data_image.shape[1])
        # no_rgb=int(data_image.shape[2])
        no_pixels = self.paramsDict['imgWNew'] * self.paramsDict['imgHNew']  # set_x*set_y
        img_data = numpy.zeros(
            [min_no_images*len(participant_index)*len(self.paramsDict['pose_index']), no_pixels])
        img_label_data = []
        # cv2.imshow("test", data_image)
        # cv2.waitKey(50)
        countPos = 0
        for count_pose, current_pose in enumerate(self.paramsDict['pose_index']):
            for count_participant, current_participant in enumerate(participant_index):
                for current_image in range(min_no_images):
                    current_image_path = os.path.join(os.path.join(root_data_dir,
                                                                   participant_index[count_participant] +
                                                                   self.paramsDict['pose_index'][count_pose] + "/" +
                                                                   data_file_database[
                                                                       participant_index[count_participant]][
                                                                       self.paramsDict['pose_index'][count_pose]][
                                                                       current_image][2]))
                    data_image = cv2.imread(current_image_path)
                    # Check image is the same size if not... cut or reject
                    if data_image.shape[0] < set_x or data_image.shape[1] < set_y:
                        logging.error("Image too small... EXITING:")
                        logging.error("Found image with dimensions" + str(data_image.shape))
                        sys.exit(0)
                    if data_image.shape[0] > set_x or data_image.shape[1] > set_y:
                        logging.warning("Found image with dimensions" + str(data_image.shape))
                        logging.warning("Image too big cutting to: x=" + str(set_x) + " y=" + str(set_y))
                        data_image = data_image[:set_x, :set_y]
                    data_image = cv2.resize(data_image, (self.paramsDict['imgWNew'], self.paramsDict['imgHNew']))  # New
                    data_image = cv2.cvtColor(data_image, cv2.COLOR_BGR2GRAY)
                    # Data is flattened into single vector (inside matrix of all images) -> (from images)        
                    img_data[countPos, :] = data_image.flatten()
                    countPos += 1
                    # Labelling with participant
                    img_label_data.append(participant_index[count_participant])

        self.Y = img_data
        self.L = img_label_data
        return self.Y.shape[1]

    def processLiveData(self, dataList, thisModel, verbose, additionalData=dict()):
        """
            Method which receives a list of data frames and outputs a classification if available or 'no_classification' if it is not

            Args:
                dataList: List of dataFrames collected. Length of list is variable.
                thisModel: List of models required for testing.
                verbose : Boolean turning logging to stdout on or off.
                additionalData : Dictionary containing additional data required for classification to occur.
            Returns:
               String with result of classification, likelihood of the classification, and list of frames with the latest x number of frames popped where x is the window length of the model. Classification result can be string `'None'` if the classification is unknown or message is invalid or `None` if a different error occurs.
        """

        logging.info('process live data')
        logging.info(len(dataList))

        imgH = thisModel[0].paramsDict['imgH']
        imgW = thisModel[0].paramsDict['imgW']
        imgHNew = thisModel[0].paramsDict['imgHNew']
        imgWNew = thisModel[0].paramsDict['imgWNew']
        numFaces = len(dataList)

        imageArray = numpy.zeros((imgH, imgW, 3), dtype=numpy.uint8)
        yarpImage = yarp.ImageRgb()
        yarpImage.resize(imgH, imgW)
        yarpImage.setExternal(imageArray, imageArray.shape[1], imageArray.shape[0])

        # images = numpy.zeros((numFaces, imgHNew * imgWNew), dtype=numpy.uint8)
        labels = [None]*numFaces
        likelihoods = [None]*numFaces

        if numFaces > 0:
            # average all faces
            for i in range(numFaces):
                logging.info('iterating' + str(i))
                yarpImage.copy(dataList[i])
                imageArrayOld = cv2.resize(imageArray, (imgHNew, imgWNew))
                imageArrayGray = cv2.cvtColor(imageArrayOld, cv2.COLOR_BGR2GRAY)
                instance = imageArrayGray.flatten()[None, :]
                logging.info(instance.shape)
                logging.info("Collected face: " + str(i))
                logging.info('testing enter')
                [labels[i], likelihoods[i]] = SAMTesting.testSegment(thisModel, instance, verbose, None)
                logging.info('testing leave')
            logging.info('combine enter')
            finalClassLabel, finalClassProb = SAMTesting.combineClassifications(thisModel, labels, likelihoods)
            logging.info('combine ready')
            logging.info('finalClassLabels ' + str(finalClassLabel))
            logging.info('finalClassProbs ' + str(finalClassProb))
            return finalClassLabel, finalClassProb, []
        else:
            return [None, 0, None]

    def formatGeneratedData(self, instance):
        """
        Method to transform a generated instance from the model into a Yarp formatted output.

        Args:
            instance: Feature vector returned during generation of a label.

        Returns:
            Yarp formatted output for instance.
        """
        # normalise image between 0 and 1
        yMin = instance.min()
        instance -= yMin
        yMax = instance.max()
        instance /= yMax
        instance *= 255
        instance = instance.astype(numpy.uint8)
        instance = numpy.reshape(instance, (self.paramsDict['imgHNew'], self.paramsDict['imgWNew']))

        # convert image into yarp rgb image
        yarpImage = yarp.ImageMono()
        yarpImage.resize(self.paramsDict['imgHNew'], self.paramsDict['imgWNew'])
        instance = instance.astype(numpy.uint8)
        yarpImage.setExternal(instance, instance.shape[1], instance.shape[0])

        return yarpImage

