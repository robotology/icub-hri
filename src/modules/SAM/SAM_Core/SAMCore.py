# """"""""""""""""""""""""""""""""""""""""""""""
# The University of Sheffield
# WYSIWYD Project
#
# The core of the Synthetic Autobiographical Memory (SAM) system.
# The core is built upon Latent Feature Models (LFMs) and implements
# cognitive memory properties, such as recall, pattern completion, compression etc.
#
# The Core is accompanied by peripherals, currently implemented as Drivers.
# Drivers facilitate the communication of sensory modalities and the Core.
# See Driver.py for this.
#
# Created:  2015
#
# @authors: Andreas Damianou
#
# """"""""""""""""""""""""""""""""""""""""""""""


import GPy
import numpy as np
import matplotlib.cm as cm
import itertools
import pylab as pb
import cPickle as pickle
from scipy.spatial import distance
import operator
import os


## @{
## \defgroup icubclient_SAM_Core SAM Core
## This Module implements the core functionality to create Gaussian Process Latent Feature Models
## \ingroup icubclient_SAM_source
## @}
## \ingroup icubclient_SAM_Core
class LFM(object):
    """
    SAM based on Latent Feature Models.
    """
    def __init__(self):
        """
        Initalise the Latent Feature Models.
        """
        self.type = []
        self.model = []
        self.observed = None
        self.inputs = None
        self.__num_views = None
        self.Q = None
        self.N = None
        self.num_inducing = None
        self.namesList = None
        self.Ylist = None

    def store(self, observed, inputs=None, Q=None, kernel=None, num_inducing=None, init_X='PCA'):
        """Store events.

            Description:
                Read in the args observed and inputs and configure the LFM model for training or recollection.

            Args:
                observed: An `(N x D)` matrix, where `N` is the number of points and `D` the number of features needed to describe each point.
                inputs: A `(N x Q)` matrix, where `Q` is the number of features per input. Leave `"None"` for unsupervised learning.
                Q: Leave `None` for supervised learning (`Q` will then be the dimensionality of inputs). Otherwise, specify with an integer `Q` the dimensionality (number of features) for the compressed space that acts as "latent" inputs.
                kernel: For the GP. Can be left as `"None"` for the default kernel.
                num_inducing: Integer of how many inducing points to use. Inducing points are a fixed number of variables through which all memory is filtered, to achieve full compression. E.g. it can correspond to the number of neurons. This is not absolutely fixed, but it also doesn't grow necessarily proportionally to the data, since synapses can make more complicated combinations of the existing neurons. The GP is here playing the role of "synapses", by learning non-linear and rich combinations of the inducing points.
                init_X: Initialisation method for model output. String either `PCA` or `PPCA`. Default= `PCA`. Initialisation uses `PPCA` when `PCA`

            Returns:
                None
        """
        assert(isinstance(observed, dict))
        self.observed = observed
        self.__num_views = len(self.observed.keys())
        self.Q = Q
        # self.D = observed.shape[1]
        self.N = observed[observed.keys()[0]].shape[0]
        self.num_inducing = num_inducing
        if num_inducing is None:
            self.num_inducing = self.N
        if inputs is None:
            if self.Q is None:
                self.Q = 2  # self.D
            if self.__num_views == 1:
                assert(self.type == [] or self.type == 'bgplvm')
                self.type = 'bgplvm'
            else:
                assert(self.type == [] or self.type == 'mrd')
                self.type = 'mrd'
        else:
            assert(self.type == [] or self.type == 'gp')
            assert(self.__num_views == 1)
            self.Q = inputs.shape[1]
            self.type = 'gp'
            self.inputs = inputs

        if kernel is None:
            kernel = GPy.kern.RBF(self.Q, ARD=True) + GPy.kern.Bias(self.Q) + GPy.kern.White(self.Q)
            
        if self.type == 'bgplvm':
            Ytmp = self.observed[self.observed.keys()[0]]
            pcaFailed = False
            if init_X == 'PCA':
                try:
                    self.model = GPy.models.BayesianGPLVM(Ytmp, self.Q, kernel=kernel, num_inducing=self.num_inducing)
                except ValueError:
                    pcaFailed = True
                    print "Initialisation with PCA failed. Initialising with PPCA..."
            elif init_X == 'PPCA' or pcaFailed:
                print "Initialising with PPCA..."
                Xr = GPy.util.linalg.ppca(Ytmp, self.Q, 2000)[0]
                Xr -= Xr.mean(0)
                Xr /= Xr.std(0)
                self.model = GPy.models.BayesianGPLVM(Ytmp, self.Q, kernel=kernel, num_inducing=self.num_inducing, X=Xr)
            self.model['.*noise'] = Ytmp.var() / 100.
        elif self.type == 'mrd':
            # Create a list of observation spaces (aka views)
            self.Ylist = []
            self.namesList = []
            for k in self.observed.keys():
                self.Ylist = [self.Ylist, self.observed[k]]
                self.namesList = [self.namesList, k]
            self.Ylist[0] = self.Ylist[0][1]
            self.namesList[0] = self.namesList[0][1]
            pcaFailed = False
            if init_X == 'PCA':
                try:
                    self.model = GPy.models.MRD(self.Ylist, input_dim=self.Q, num_inducing=self.num_inducing,
                                                kernel=kernel, initx="PCA_single", initz='permute')
                except ValueError:
                    pcaFailed = True
                    print "Initialisation with PCA failed. Initialising with PPCA..."
            elif init_X == 'PPCA' or pcaFailed:
                print "Initialising with PPCA..."
                from GPy.util.initialization import initialize_latent
                Xr = np.zeros((self.Ylist[0].shape[0], self.Q))
                for qs, Y in zip(np.array_split(np.arange(self.Q), len(self.Ylist)), self.Ylist):
                    try:
                        x, frcs = initialize_latent('PCA', len(qs), Y)
                    except ValueError:
                        x = GPy.util.linalg.ppca(Y, len(qs), 2000)[0]
                    Xr[:, qs] = x
                Xr -= Xr.mean()
                Xr /= Xr.std()
                self.model = GPy.models.MRD(self.Ylist, input_dim=self.Q, num_inducing=self.num_inducing, kernel=kernel, initx="PCA_single", initz='permute', X=Xr)
            self.model['.*noise'] = [yy.var() / 100. for yy in self.model.Ylist]
        elif self.type == 'gp':
            self.model = GPy.models.SparseGPRegression(self.inputs, self.observed[self.observed.keys()[0]], kernel=kernel, num_inducing=self.num_inducing)
        
        self.model.data_labels = None
        self.model.textLabelPts = dict()

    # def _init_latents():
    #    from GPy.util.initialization import initialize_latent
    #    X, fracs = initialize_latent(init, input_dim, Y)

    def add_labels(self, labels):
        """Add labels to observations.

        Description:
            If observables are associated with labels, they can be added here. Labels has to be a matrix of size N x K, where K is the total number of different labels. If e.g. the i-th row of L is [1 0 0] (or [1 -1 -1]) then this means that there are K=3 different classes and the i-th row of the observables belongs to the first class.

        Args:
            labels: list of strings containing the labels for the observations

        Returns:
            None
        """
        if len(labels.shape) == 1 or labels.shape[1] == 1:
            self.model.data_labels = labels
        else:
            print "Warning: labels assumed to be in 1-of-K encoding!"
            self.model.data_labels = np.argmax(labels, 1)[:, None]

    def learn(self, optimizer='bfgs', max_iters=1000, init_iters=300, verbose=True):
        """
        Learn the model (analogous to "forming synapses" after perceiving data).

        Args:
            optimizer: String with the requested optimiser taken from a the list of available scipy optimisers.
            max_iters: Integer with the maximum number of training iterations for the second phase of training the model.
            init_iters: Integer with the maximum number of training iterations for the first phase of training.
            verbose: Boolean to turn logging to stdout on or off.

        Returns:
            None
        """
        if self.type == 'bgplvm' or self.type == 'mrd':
            self.model['.*noise'].fix()
            self.model.optimize(optimizer, messages=verbose, max_iters=init_iters)
            self.model['.*noise'].unfix()
            self.model['.*noise'].constrain_positive()
        
        self.model.optimize(optimizer, messages=verbose, max_iters=max_iters)
        self.check_snr()

        for j in list(np.unique(self.model.data_labels)):
            self.model.textLabelPts[j] = [l for l, k in enumerate(self.model.data_labels) if k[0] == j]

    def check_snr(self, warningEnable=True, messages=True):
        """Checks the signal to noise ratio(SNR) of the trained model.

        Description:
            Provides an indicator of successful learning by looking at the variance distribution of the model.

        Args:
            warningEnable: Boolean to switch warnings on or off in the case of a low SNR.
            messages: Boolean to turn output to stdout on or off.
        Returns:
            None
        """
        if self.type == 'bgplvm':
            snr = self.model.Y.var()/self.model.Gaussian_noise.variance.values[0]

            if messages:
                print('# SNR: ' + str(snr))
            if warningEnable and snr < 8:
                print(' WARNING! SNR is small!')
        elif self.type == 'mrd':
            snr = []
            for i in range(len(self.model.bgplvms)):
                snr.append(self.model.bgplvms[i].Y.var()/self.model.bgplvms[i].Gaussian_noise.variance.values[0])
                if messages:
                    print('# SNR view ' + str(i) + ': ' + str(snr[-1]))
                if warningEnable and snr[-1] < 8:
                    print(' WARNING! SNR for view ' + str(i) + ' is small!!')
        else:
            snr = None
        return snr
        
    def visualise(self, which_indices=None, plot_scales=True):
        """
        Show the internal representation of the memory.

        Description:
            Creates a 2D plot showing the mean and variance distribution of the model.

        Args:
            which_indices: Tuple of two integers that specify which indices of the `Q` indices that make up the model are to be plotted.
            plot_scales: Boolean to switch scale labelling on or off in the plots.

        Returns:
            None
        """
        # if self.type == 'bgplvm' and which_indices is None:
        #    which_indices = most_significant_input_dimensions(self.model,None)
        # if self.type == 'mrd' and which_indices is None:
        #    # Assume that labels modality is always the last one!!
        #    which_indices = most_significant_input_dimensions(self.model.bgplvms[-1],None)
        if self.type == 'bgplvm' or self.type == 'mrd':
            if self.model.data_labels is not None:
                ret = self.model.plot_latent(labels=self.model.data_labels, which_indices=which_indices)
            else:
                ret = self.model.plot_latent(which_indices=which_indices)
        elif self.type == 'gp':
            ret = self.model.plot()
        if self.type == 'mrd' and plot_scales:
            ret2 = self.model.plot_scales()

        # if self.type == 'mrd':
        #    ret1 = self.model.X.plot("Latent Space 1D")
        #    ret2 = self.model.plot_scales("MRD Scales")
        
        return ret

    def visualise_interactive(self, dimensions=(20, 28), transpose=True, order='F', invert=False, scale=False,
                              colorgray=True, view=0, which_indices=(0, 1)):
        """Interactive plot of the model.

        Description:
            Show the internal representation of the memory and allow the user to interact with it to map samples/points from the compressed space to the original output space.

        Args:
            dimensions: Tuple of integers describing the dimensions that the image needs to be transposed to for display.
            transpose: Boolean whether to transpose the image before display.
            order: Boolean whether array is in Fortan ordering ('F') or Python ordering ('C').
            invert: Boolean whether to invert the pixels or not.
            scale: Boolean whether to scale the image or not.
            colorgray: Boolean whether to plot in grayscale or not.
            view: Integer in the case of MRD models which describes the view to be plotted.
            which_indices: Tuple of two integers that specify which indices of the `Q` indices that make up the model are to be plotted.

        Returns:
            None
        """

        if self.type == 'bgplvm':
            ax = self.model.plot_latent(which_indices)
            y = self.model.Y[0, :]
            # dirty code here
            if colorgray:
                data_show = GPy.plotting.matplot_dep.visualize.image_show(y[None, :], dimensions=dimensions, transpose=transpose, order=order, invert=invert, scale=scale, cmap = cm.Greys_r)
            else:
                data_show = GPy.plotting.matplot_dep.visualize.image_show(y[None, :], dimensions=dimensions, transpose=transpose, order=order, invert=invert, scale=scale)
            lvm = GPy.plotting.matplot_dep.visualize.lvm(self.model.X.mean[0, :].copy(), self.model, data_show, ax)
            raw_input('Press enter to finish')
        elif self.type == 'mrd':
            """
            NOT TESTED!!!
            """
            ax = self.model.bgplvms[view].plot_latent(which_indices)
            y = self.model.bgplvms[view].Y[0, :]
            # dirty code here
            if colorgray:
                data_show = GPy.plotting.matplot_dep.visualize.image_show(y[None, :], dimensions=dimensions, transpose=transpose, order=order, invert=invert, scale=scale, cmap = cm.Greys_r)
            else:
                data_show = GPy.plotting.matplot_dep.visualize.image_show(y[None, :], dimensions=dimensions, transpose=transpose, order=order, invert=invert, scale=scale)
            lvm = GPy.plotting.matplot_dep.visualize.lvm(self.model.bgplvms[view].X.mean[0, :].copy(), self.model.bgplvms[view], data_show, ax)
            raw_input('Press enter to finish')

    def recall(self, locations):
        """
        Recall stored events.

        Description:
            This is closely related to performing pattern pattern_completion but given "training" data.

        Args:
             locations: Integer which is the index of the stored event.

        Returns:
            A `(Dx1)` numpy array containing the data of a training point as reconstructed by the model.
        """
        if locations == -1:
            locations = range(self.N)
        if self.type == 'bgplvm' or self.type == 'gp':
            return self.model.Y[locations, :].values
        elif self.type == 'mrd':
            return self.model.bgplvms[0].Y[locations, :].values

    def pattern_completion(self, test_data, view=0, verbose=False, visualiseInfo=None, optimise=100):
        """Recall novel events

        Description:
            In the case of supervised learning, pattern completion means that we give new inputs and infer their corresponding outputs. In the case of unsupervised learning, pattern completion means that we give new outputs and we infer their corresponding "latent" inputs, ie the internal compressed representation of the new outputs in terms of the already formed "synapses".

        Args:
            test_data : A `(Dx1)` numpy array containing the feature vector for which you would like to obtain the closest neighbour.
            view : Integer which is the index for the view of the MRD model that will be used for pattern completion.
            verbose : Boolean switching logging to stdout on and off.
            visualiseInfo: Plot object returned by visualiseInfo. If present, plot the location of the pattern completed point. If none, no plotting.
            optimise : Integer number of optimisation iterations when performing pattern completion.

        Returns:
            A `(Qx1)` numpy array with the predicted mean, a `(Qx1)` numpy array with the predicted variance, a plot object with plotted point and an inference object returned by optimiser.
        """
        if self.type == 'bgplvm':
            # tmp = self.model.infer_newX(test_data)[0]
            # pred_mean = tmp.mean
            # pred_variance = tmp.variance #np.zeros(pred_mean.shape)
            tmp = self.model.infer_newX(test_data, optimize=False)[1]
            if optimise != 0:
                tmp.optimize(max_iters=optimise, messages=verbose)
            pred_mean = tmp.X.mean
            pred_variance = tmp.X.variance
        elif self.type == 'mrd':
            tmp = self.model.bgplvms[view].infer_newX(test_data, optimize=False)[1]
            if optimise != 0:
                tmp.optimize(max_iters=optimise, messages=verbose)
            pred_mean = tmp.X.mean
            pred_variance = tmp.X.variance
        elif self.type == 'gp':
            tmp = []
            pred_mean, pred_variance = self.model.predict(test_data)

        if (self.type == 'mrd' or self.type == 'bgplvm') and visualiseInfo is not None:
            ax = visualiseInfo['ax']
            inds0, inds1 = most_significant_input_dimensions(self.model, None)
            pp = ax.plot(pred_mean[:, inds0], pred_mean[:, inds1], 'om', markersize=11, mew=11)
            pb.draw()
        else:
            pp = None
        
        return pred_mean, pred_variance, pp, tmp

    def pattern_completion_inference(self, y, target_modality=-1):
        """Pattern completion wrapper.

        Description:
            1) First, we do normal pattern completion, where given an output y, out map to the memory space to get a test memory x*.
            2) Now the test memory x* is compared with stored memories. This allows us to infer the label of x*. If the labels are given in another modality (by default in the last one), then we return the label from that modality (careful, The encoding might be 1-of-K, e.g. -1 1 -1 -> 2 and also noise might exist). Instead, if the labels are not given in another modality (completely unsupervised learning), then we just return the index to the most similar training memory.

        Args:
            y : A `(Dx1)` numpy array containing the feature vector for which you would like to obtain the closest neighbour.
            target_modality : Integer which is the index for the view of the MRD model that will be used for pattern completion.

        Returns:
            Inference object returned by optimiser containing multi-dimensional mean and variance of nearest neighbour.
        """
        # Returns the predictive mean, the predictive variance and the axis (pp) of the latent space backwards mapping.            
        ret = self.pattern_completion(y)
        mm = ret[0]
        post = ret[3]        
        # find nearest neighbour of mm and model.X
        dists = np.zeros((self.model.X.shape[0], 1))

        for j in range(dists.shape[0]):
            dists[j, :] = distance.euclidean(self.model.X.mean[j, :], mm[0].values)
        nn, min_value = min(enumerate(dists), key=operator.itemgetter(1))
        if self.type == 'mrd':
            ret = self.model.bgplvms[target_modality].Y[nn, :]
        elif self.type == 'bgplvm':
            ret = nn  # self.model.data_labels[nn]
        return ret

    def fantasy_memory(self, X, view=0):
        """Generating novel outputs.

        Description:
            The opposite of pattern completion. Instead of finding a memory from an output, here we find an output from a (possibly fantasy) memory. Here, fantasy memory is a memory not existing in the training set, found by interpolating or sampling in the memory space.

        Args:
            X: A `(Qx1)` numpy array with the location of the `Q` dimensional model that is to be generated.
            view: Integer which is the index for the view of the MRD model that will be used to generate the fantasy_memory.
        Returns:
            A `(Qx1)` numpy array with the predicted mean and a `(Qx1)` numpy array with the predicted variance.
        """
        if self.type == 'mrd':
            pred_mean, pred_variance = self.model.bgplvms[view].predict(X)
        elif self.type == 'bgplvm':
            pred_mean, pred_variance = self.model.predict(X)
        elif self.type == 'gp':
            pred_mean, pred_variance = self.model.predict(X)
        return pred_mean, pred_variance

    def familiarity(self, Ytest, ytrmean=None, ytrstd=None, optimise=100):
        """Familiarity testing.

        Description:
            This function tests the familiarity/similarity of an input with the inputs used to train the model.

        Args:
            Ytest : A `(Dx1)` numpy array whose familiarity/similarity is tested with trained outputs of the model.
            ytrmean : A `(Dx1)` numpy array with the mean of the training inputs.
            ytrstd :  A `(Dx1)` numpy array with the variance of the training inputs.
            optimise : Integer number of optimisation iterations when performing pattern completion.

        Returns:
            A float with a measure of familiarity for Ytest with the current model.
        """
        assert(self.type == 'bgplvm')

        N = Ytest.shape[0]
        if ytrmean is not None:
            Ytest -= ytrmean
            Ytest /= ytrstd

        from SAM.SAM_Core.svi_ratio import SVI_Ratio
        s = SVI_Ratio()
        _, _, _, qX = self.pattern_completion(Ytest, verbose=False, optimise=optimise)
        qX = qX.X

        ll = 0
        for i in range(N):
            ll += s.inference(self.model.kern, qX[i, :][None, :], self.model.Z, self.model.likelihood,
                              Ytest[i, :][None, :], self.model.posterior)[0]
        ll /= N
        return ll

    def __get_latent__(self):
        """ Return number of latent dimensions.

            Description:
                Convenience function to return the number of latent dimensions.

            Args:
                None.

            Returns:
                Integer with the number of latent dimensions of the model.
        """
        if self.type == 'bgplvm':
            numLatentDimensions = self.model.X.mean
        elif self.type == 'mrd':
            numLatentDimensions = self.model.bgplvms[0].X.mean
        else:
            print('No latent space for this type of model.')
            numLatentDimensions = None
        return numLatentDimensions


## \ingroup icubclient_SAM_Core
def save_model(mm, fileName='m_serialized.txt'):
    """ Save serialised model.

        Args:
            mm : Model object to save.
            fileName : String with the filename of saved model.
        Returns:
            None
    """
    #mPruned = mm.getstate() # TODO (store less stuff)
    output = open(fileName, 'wb')
    #pickle.dump(mPruned, output)
    pickle.dump(mm, output)
    output.close()

## \ingroup icubclient_SAM_Core
def load_model(fileName='m_serialized.txt'):
    """ Load serialised model.

        Args:
            fileName : String with the filename of model to load.
        Returns:
            SAMObject Model
    """
    mm = pickle.load(open(fileName, 'r'))
    return mm

## \ingroup icubclient_SAM_Core
def save_pruned_model(mm, fileName='m_pruned', economy=False, extraDict=dict()):
    """Save a pruned model

    Description:
            Save a trained model after pruning things that are not needed to be stored. Economy set to `True` will trigger a storing which creates much smaller files. See the load_pruned_model discussion on what this means in terms of restrictions.

    Args:
        mm : Model object to save.
        fileName : String with the filename of saved model.
        economy : Boolean to enable or disable economy saving.
        extraDict : Dictionary with parameters that are requested to be saved which are not in the default saved parameters but are required when loading the model for interaction.

    Returns:
        None
    """
    SAMObjPruned=dict()
    SAMObjPruned['type'] = mm.type
    if mm.model:
        SAMObjPruned['textLabelPts'] = mm.model.textLabelPts
    # SAMObjPruned['observed'] = mm.observed # REMOVE
    # SAMObjPruned['inputs'] = mm.inputs
#    SAMObjPruned['__num_views'] = mm.__num_views
    SAMObjPruned['__num_views'] = None
    SAMObjPruned['Q'] = mm.Q
    SAMObjPruned['N'] = mm.N
    SAMObjPruned['num_inducing'] = mm.num_inducing
    SAMObjPruned['namesList'] = mm.namesList
    SAMObjPruned['kernelString'] = mm.kernelString
    SAMObjPruned.update(extraDict)

    # if economy:
    #     SAMObjPruned['modelPath'] = fileName + '_model.h5'
    #     #if file exists delete
    #     if(os.path.isfile(SAMObjPruned['modelPath'])):
    #         os.remove(SAMObjPruned['modelPath'])
    #     mm.model.save(SAMObjPruned['modelPath'])
    # else:
    #     SAMObjPruned['modelPath'] = fileName + '_model.pickle'
    #     mm.model.pickle(SAMObjPruned['modelPath'])
    folderPath = os.path.join('/', *fileName.split('/')[:-1])
    fileName = fileName.split('/')[-1]

    if economy:
        SAMObjPruned['modelPath'] = fileName + '_model.h5'
        # if file exists delete
        if os.path.isfile(os.path.join(folderPath, SAMObjPruned['modelPath'])):
            os.remove(os.path.join(folderPath, SAMObjPruned['modelPath']))
        if mm.model:
            mm.model.save(os.path.join(folderPath, SAMObjPruned['modelPath']))
    else:
        SAMObjPruned['modelPath'] = fileName + '_model.pickle'
        mm.model.pickle(os.path.join(folderPath, SAMObjPruned['modelPath']))

    output = open(os.path.join(folderPath, fileName) + '.pickle', 'wb')
    pickle.dump(SAMObjPruned, output)
    output.close()

## \ingroup icubclient_SAM_Core
def load_pruned_model(fileName='m_pruned', economy=False, m=None):
    """Load a pruned model

        Description:
            Load a trained model. If economy is set to `True`, then a not-None initial model m is needed. This model needs to be created exactly as the one that was saved (so, it is demo specific!) and in this case calling the present function will set its parameters (meaning that you still need to create a model but don't need to optimize it).

        Args:
            fileName : String with the filename of the model to load.
            economy : Boolean to indicate whether an economy object is being loaded or not.
            m : Model object into which the data to be loaded is to be stored in. If left at `None` model will be loaded into a default model initialisation.

        Returns:
            SAMObject model
        """
    folderPath = os.path.join('/', *fileName.split('/')[:-1])
    SAMObjPruned = pickle.load(open(fileName + '.pickle', 'rb'))
    SAMObject = LFM()
    if economy:
        assert m is not None
        import tables
        f = tables.open_file(os.path.join(fileName+'_model.h5'), 'r')
        m.param_array[:] = f.root.param_array[:]
        f.close()
        m._trigger_params_changed()
        SAMObject.model = m
    else:
        with open(SAMObjPruned['modelPath'], 'rb') as f:
            print "Loading file: " + str(f)
            SAMObject.model = pickle.load(f)
        # TODO: The following is supposed to update the model, but maybe not. Change...
    # LB get error here using MRD
    # SAMObject.model.update_toggle()
    # SAMObject.model.update_toggle()
    
    SAMObject.type = SAMObjPruned['type'] 
    # SAMObject.observed = SAMObjPruned['observed'] 
    # SAMObject.inputs = SAMObjPruned['inputs']
    SAMObject.model.textLabelPts = SAMObjPruned['textLabelPts']
    SAMObject.__num_views = SAMObjPruned['__num_views'] 
    SAMObject.Q = SAMObjPruned['Q'] 
    SAMObject.N = SAMObjPruned['N'] 
    SAMObject.num_inducing = SAMObjPruned['num_inducing'] 
    SAMObject.namesList = SAMObjPruned['namesList']

    return SAMObject

## \ingroup icubclient_SAM_Core
def most_significant_input_dimensions(model):
    """ Determine the most descriptive output dimensions.

    Description:
        Helper function to determine which dimensions should be plotted based on the relevance weights.

    Args:
        model: Model object to be assessed.

    Returns:
        Integer indicating the most descriptive dimension and an integer indicating the second most descriptive dimension.
    """
    if model.input_dim == 1:
        input_1 = 0
        input_2 = None
    if model.input_dim == 2:
        input_1, input_2 = 0, 1
    else:
        try:
            input_1, input_2 = np.argsort(model.input_sensitivity())[::-1][:2]
        except:
            raise ValueError("cannot automatically determine which dimensions to plot, please pass 'which_indices'")

    return input_1, input_2
