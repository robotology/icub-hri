# Copyright (c) 2015, Andreas Damianou

import matplotlib
matplotlib.use('TkAgg')
import numpy as np
import matplotlib.pyplot as plt

## \defgroup icubclient_SAM_source SAM
## Synthetic Autobiographical Memory for face recognition and action recognition
## @{
## @ingroup icubclient_modules
from . import SAM_Core
from . import SAM_Drivers
from . import SAM_Tests
## @}

# plt.ion()

default_seed = 123344
