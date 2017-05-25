FROM nvidia/cuda:8.0-cudnn5-devel-ubuntu16.04

# If host is running squid-deb-proxy on port 8000, populate /etc/apt/apt.conf.d/30proxy
# By default, squid-deb-proxy 403s unknown sources, so apt shouldn't proxy ppa.launchpad.net
# this will cache apt-get update results for the rest of the dockerfile allowing docker to cache apt-get install commands
RUN route -n | awk '/^0.0.0.0/ {print $2}' > /tmp/host_ip.txt
RUN echo "HEAD /" | nc `cat /tmp/host_ip.txt` 8000 | grep squid-deb-proxy \
  && (echo "Acquire::http::Proxy \"http://$(cat /tmp/host_ip.txt):8000\";" > /etc/apt/apt.conf.d/30proxy) \
  && (echo "Acquire::http::Proxy::ppa.launchpad.net DIRECT;" >> /etc/apt/apt.conf.d/30proxy) \
  || echo "No squid-deb-proxy detected on docker host"

#create icub user
RUN apt-get update && \
    apt-get -y install sudo git

RUN useradd -m icub && echo "icub:icub" | chpasswd && adduser icub sudo && usermod -aG video icub && usermod -s /bin/bash icub

COPY ./keyboard /etc/default/keyboard

ARG cores=6
ENV SRC_FOLDER=/usr/local/src/robot
ARG localBash=/home/icub/.bashrc

# Clone libraries
RUN mkdir $SRC_FOLDER
WORKDIR $SRC_FOLDER
RUN git clone https://github.com/opencv/opencv.git 	
RUN git clone https://github.com/opencv/opencv_contrib.git
RUN git clone https://github.com/robotology/yarp.git
RUN git clone https://github.com/robotology/icub-main.git 
RUN git clone https://github.com/robotology/icub-contrib-common.git
RUN git clone https://github.com/kmarkus/rFSM.git
RUN git clone https://github.com/robotology/robots-configuration.git
RUN git clone https://github.com/BVLC/caffe.git
RUN git clone https://github.com/SheffieldML/GPy.git
RUN git clone https://github.com/robotology/segmentation.git
RUN git clone https://github.com/robotology/himrep.git
RUN git clone https://github.com/robotology/iol.git
RUN git clone https://github.com/robotology/stereo-vision.git
RUN git clone https://github.com/robotology/speech.git
RUN git clone https://github.com/robotology/boost-mil.git
RUN git clone https://github.com/dcam0050/human-sensing-SAM.git
RUN git clone https://github.com/robotology/kinect-wrapper.git
RUN git clone https://github.com/SheffieldML/GPyOpt.git
RUN git clone https://github.com/avin2/SensorKinect.git
RUN git clone https://github.com/pitzer/SiftGPU.git
RUN git clone https://github.com/towardthesea/karmaWYSIWYD.git

# Install icub ppa
RUN sh -c 'echo "deb http://www.icub.org/ubuntu xenial contrib/science" > /etc/apt/sources.list.d/icub.list'
RUN apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 57A5ACB6110576A6 && apt-get update && apt-get install -y wget

#Python
RUN wget https://repo.continuum.io/archive/Anaconda2-4.2.0-Linux-x86_64.sh
RUN cd $SRC_FOLDER && bash Anaconda2-4.2.0-Linux-x86_64.sh -b -p ./anaconda2 	&& 	\
	rm $SRC_FOLDER/anaconda2/lib/libgomp.so* 									&& 	\
	rm $SRC_FOLDER/anaconda2/lib/libstdc++.so* 									&& 	\
	rm $SRC_FOLDER/anaconda2/lib/libgfortran.so* 								&& 	\
	rm $SRC_FOLDER/anaconda2/lib/libQt5*										&& 	\
	rm -r $SRC_FOLDER/anaconda2/lib/cmake/Qt5*

#Install required libraries
RUN apt-get update && apt-get install -y icub-common 		\
	build-essential 	\
	cmake-curses-gui 	\
	pciutils 			\
	wget 				\
	libgtk2.0-dev 		\
	mesa-utils git 		\
	pkg-config 			\
	libavcodec-dev 		\
	libavformat-dev 	\
	libswscale-dev 		\
	libtbb2 			\
	libtbb-dev 			\
	libjpeg-dev 		\
	libpng-dev 			\
	libtiff-dev 		\
	libjasper-dev 		\
	libdc1394-22-dev 	\
	checkinstall 		\
	libopenblas-dev 	\
	liblapack-dev 		\
	liblapacke-dev 		\
	libboost-all-dev 	\
	doxygen 			\
	libeigen3-dev 		\
	bash-completion 	\
	apt-utils  			\
	vim					\
	net-tools			\
	software-properties-common \
	swig3.0				\
	liblua5.2-dev	    \
	xterm 				\
	gdb					\
	gedit				\
	libopenblas-dev		\
	libatlas-dev		\
	libboost-all-dev	\
	libprotobuf-dev 	\
	protobuf-compiler	\
	libgoogle-glog-dev	\
	libgflags-dev		\
	libleveldb-dev		\
	libhdf5-serial-dev	\
	liblmdb-dev			\
	libsnappy-dev		\
	hdf5-tools 			\
	libatlas-base-dev 	\
	libatlas3-base		\
	libglew-dev 		\
	libdevil-dev		\
	python-tk 			\
	usbutils 			\
	iputils-ping 		\
	openssh-server 		\
	libopenni-dev 		\
	bison 				\
	flex 				\
	libpq-dev			\
	libsqlite3-dev		\
	postgresql 			\
	pgadmin3			\
	expect 				\
	libasound2 			\
	libpango1.0-0 		\
	pulseaudio-utils 	\
	alsa-base 			\
	alsa-utils

ENV PYTHONPATH=$SRC_FOLDER/anaconda2/bin 
ENV PATH=$PYTHONPATH:$PATH

RUN wget https://download.sublimetext.com/sublime-text_build-3126_amd64.deb && \
	dpkg -i sublime-text_build-3126_amd64.deb && rm sublime-text_build-3126_amd64.deb

RUN cp -r opencv opencv3 && mv opencv opencv2

ARG opencv3Hash=70bbf17b133496bd7d54d034b0f94bd869e0e810
RUN cd opencv3 && git checkout -b localBranch $opencv3Hash

ARG opencv3ContribHash=86342522b0eb2b16fa851c020cc4e0fef4e010b7
RUN cd opencv_contrib && git checkout -b localBranch $opencv3ContribHash

RUN mkdir opencv3/build 	&& cd opencv3/build 	&& mkdir ./install 							&& \
	cmake .. 	-DCMAKE_BUILD_TYPE=Release 									\
				-DBUILD_PERF_TESTS=OFF 										\
				-DCMAKE_INSTALL_PREFIX=./install 							\
				-DBUILD_TESTS=OFF 											\
				-DBUILD_opencv_cnn_3dobj=OFF 								\
				-DCUDA_ARCH_BIN="6.0 6.1" 									\
				-DCUDA_FAST_MATH=ON 										\
				-DOPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules 	\
				-DWITH_CUBLAS=ON 											\
				-DWITH_QT=ON 												\
				-DWITH_TIFF=OFF 											\
				-DWITH_TBB=ON 												\
				-DBUILD_opencv_tracking=ON 									\
				-DBUILD_opencv_dnn=ON										\
				-DINSTALL_PYTHON_EXAMPLES=ON 								\
				-DINSTALL_TESTS=ON 											\
				-DBUILD_opencv_hdf=OFF 										\
				-DPYTHON2_NUMPY_INCLUDE_DIRS=$SRC_FOLDER/anaconda2/lib/python2.7/site-packages/numpy/core/include 	\
				-DPYTHON2_LIBRARY=$SRC_FOLDER/anaconda2/lib/libpython2.7.so 										\
				-DPYTHON2_PACKAGES_PATH=$SRC_FOLDER/anaconda2/lib/python2.7/site-packages 						&& 	\
	make -j$cores && make -j$cores install

ENV OPENCV_DIR=$SRC_FOLDER/opencv3 OPENCV_ROOT=$SRC_FOLDER/opencv3/build/install 
ENV OPENCV_LIB=$OPENCV_ROOT/lib OPENCV_BIN=$OPENCV_ROOT/bin \
	OPENCV_INCLUDE=$OPENCV_ROOT/include OPENCV_PYTHONLIB=$OPENCV_DIR/build/lib
ENV PATH=$OPENCV_BIN:$OPENCV_INCLUDE:$PATH \
	LD_LIBRARY_PATH=$OPENCV_LIB:$OPENCV_PYTHONLIB:$LD_LIBRARY_PATH

# # YARP
ARG yarpHash=cca890b8ae3ec14fc7cadab0821bcd1a09181da1
RUN	cd yarp && git checkout -b localBranch $yarpHash

ADD yarp.i yarp/bindings

RUN mkdir yarp/build && cd yarp/build 									&& 	\
	cmake .. 	-DCMAKE_BUILD_TYPE=Release 									\
				-DCREATE_DEVICE_LIBRARY_MODULES=ON 							\
				-DCMAKE_INSTALL_PREFIX=./install 							\
				-DCREATE_GUIS=ON 											\
				-DCREATE_LIB_MATH=ON 										\
				-DCREATE_OPTIONAL_CARRIERS=ON								\
				-DYARP_COMPILE_BINDINGS=ON									\
				-DCREATE_CSHARP=ON 											\
				-DCREATE_LUA=ON 											\
				-DCREATE_PYTHON=ON 											\
				-DENABLE_yarpcar_bayer=ON 									\
				-DENABLE_yarpcar_mjpeg=ON 									\
				-DENABLE_yarpmod_opencv_grabber=ON 							\
				-DENABLE_yarpmod_serial=ON 									\
				-DENABLE_yarpmod_serialport=ON 								\
				-DYARP_USE_PYTHON_VERSION=2								 &&	\
	cmake .. 	-DCMAKE_BUILD_TYPE=Release 									\
				-DCREATE_DEVICE_LIBRARY_MODULES=ON 							\
				-DCMAKE_INSTALL_PREFIX=./install 							\
				-DCREATE_GUIS=ON 											\
				-DCREATE_LIB_MATH=ON 										\
				-DCREATE_OPTIONAL_CARRIERS=ON								\
				-DYARP_COMPILE_BINDINGS=ON									\
				-DCREATE_CSHARP=ON 											\
				-DCREATE_LUA=ON 											\
				-DCREATE_PYTHON=ON 											\
				-DENABLE_yarpcar_bayer=ON 									\
				-DENABLE_yarpcar_mjpeg=ON 									\
				-DENABLE_yarpmod_opencv_grabber=ON 							\
				-DENABLE_yarpmod_serial=ON 									\
				-DENABLE_yarpmod_serialport=ON 								\
				-DYARP_USE_PYTHON_VERSION=2								&& 	\
	make -j$cores && make install 

ENV LUA_CPATH=$SRC_FOLDER/yarp/build/lib/lua/?.so  YARP_ROOT=$SRC_FOLDER/yarp
ENV YARP_DIR=$YARP_ROOT/build/install
ENV YARP_LIB=$YARP_DIR/lib \
	YARP_BIN=$YARP_DIR/bin \
	YARP_ROBOT_NAME=iCubSheffield01
ENV PATH=$YARP_BIN:$LUA_CPATH:$PATH \
	LD_LIBRARY_PATH=$YARP_LIB:$LD_LIBRARY_PATH

# ROBOTS CONFIGURATION
ARG robotConfigHash=a8e47be03b88a0dd2bc7609ab5ce6bfdf29a3c5c
RUN cd robots-configuration &&	git checkout -b localBranch $robotConfigHash

# ICUB MAIN
ARG icubMainHash=a5e94059e899c4fd6994d007dc7bdab9595a9111
RUN cd icub-main  && git checkout -b localBranch $icubMainHash

RUN mkdir icub-main/build 	&& 	cd icub-main/build 			&& 	\
	cmake .. -DCMAKE_BUILD_TYPE=Release							\
			 -DICUB_USE_GTK2=OFF 								\
			 -DCMAKE_INSTALL_PREFIX=./install 					\
			 -DENABLE_icubmod_gazecontrollerclient=ON 			\
			 -DENABLE_icubmod_cartesiancontrollerclient=ON 		\
			 -DENABLE_icubmod_cartesiancontrollerserver=ON 	&&	\
	make -j$cores && make -j$cores install

ENV ICUB_ROOT=$SRC_FOLDER/icub-main 
ENV ICUB_DIR=$ICUB_ROOT/build/install
ENV ICUB_LIB=$ICUB_DIR/lib 	\
	ICUB_BIN=$ICUB_DIR/bin 	\
	ICUB_ROBOTNAME=$YARP_ROBOT_NAME
ENV PATH=$ICUB_BIN:$PATH 	\
	LD_LIBRARY_PATH=$ICUB_LIB:$LD_LIBRARY_PATH

# ICUB CONTRIB
RUN mkdir icub-contrib-common/build && cd icub-contrib-common/build							&& 	\
	cmake .. -DCMAKE_INSTALL_PREFIX=$SRC_FOLDER/icub-contrib-common/build/install 				\
			 -DCMAKE_BUILD_TYPE=Release														&&	\
	make && make install

ENV ICUBCONTRIB_ROOT=$SRC_FOLDER/icub-contrib-common 
ENV ICUBCONTRIB_DIR=$ICUBCONTRIB_ROOT/build/install
ENV ICUBCONTRIB_BIN=$ICUBCONTRIB_DIR/bin \
	CMAKE_PREFIX_PATH=$ICUBCONTRIB_DIR
ENV PATH=$ICUBCONTRIB_DIR:$ICUBCONTRIB_BIN:$PATH

#OpenCV 2
ARG opencv2Hash=d68e3502278d6fc5a1de0ce8f7951d9961b20913
RUN cd opencv2 && git checkout -b localBranch $opencv2Hash

RUN mkdir opencv2/build 	&& cd opencv2/build 	&& mkdir ./install 	&& 	\
	cmake .. 	-DCMAKE_BUILD_TYPE=Release 									\
				-DBUILD_PERF_TESTS=OFF 										\
				-DCMAKE_INSTALL_PREFIX=./install 							\
				-DBUILD_TESTS=OFF 											\
				-DCUDA_ARCH_BIN="6.0 6.1" 									\
				-DCUDA_FAST_MATH=ON 										\
				-DWITH_CUBLAS=ON 											\
				-DWITH_QT=ON 												\
				-DWITH_TIFF=OFF 											\
				-DWITH_TBB=ON 											&& 	\
	make -j$cores && make -j$cores install

#Caffe
ARG caffeHash=68215ccc004ebd05a2209270688c26b6e4304d4a
RUN cd caffe && git checkout -b localBranch $caffeHash && mkdir build 	&& cd build	&& 	\
	cmake .. 	-DCMAKE_BUILD_TYPE=Release									\
				-DCMAKE_INSTALL_PREFIX=./install 							\
				-DOpenCV_DIR=$SRC_FOLDER/opencv2/build/install/share/OpenCV \
				-DCUDA_USE_STATIC_CUDA_RUNTIME=OFF 						&& 	\
	make -j$cores && make -j$cores install
RUN cd caffe && scripts/download_model_binary.py models/bvlc_reference_caffenet && \
	./data/ilsvrc12/get_ilsvrc_aux.sh
ENV CAFFE_ROOT=$SRC_FOLDER/caffe/build 
ENV CAFFE_DIR=$CAFFE_ROOT/install
ENV CAFFE_LIB=$CAFFE_DIR/lib 	\
	CAFFE_BIN=$CAFFE_DIR/bin
ENV PATH=$CAFFE_BIN:$PATH 		\
	LD_LIBRARY_PATH=$CAFFE_LIB:$LD_LIBRARY_PATH

#rFSM
ARG rFSMHash=26d0477053597690ab1d941f26041492ddfced9b
RUN cd rFSM && git checkout -b localBranch $rFSMHash

# Segmentation
ARG segmentationHash=0a6666ce44304a9cf52c49e4871a1fb835896200
RUN cd segmentation && git checkout -b localBranch $segmentationHash && mkdir build && \
	cd build && cmake .. -DCMAKE_INSTALL_PREFIX=$CMAKE_PREFIX_PATH && make -j$cores && make -j$cores install

#SiftGPU
ARG siftGPUHash=b46bd5b8cc5cfdc1dc163444b72c705569800b6d
RUN cd SiftGPU && git checkout -b localBranch $siftGPUHash && make clean && make

ENV SIFTGPU_ROOT=$SRC_FOLDER/SiftGPU
ENV SIFTGPU_BIN=$SIFTGPU_ROOT/bin \
	SIFTGPU_LIB=$SIFTGPU_ROOT/lib \
	SIFTGPU_INCLUDE_DIRS=$SIFTGPU_ROOT/src/SiftGPU/
ENV LUA_PATH=$SRC_FOLDER/rFSM/?.lua:$ICUBCONTRIB_DIR/share/ICUBcontrib/contexts/iol/lua/?.lua?
ENV PATH=$SIFTGPU_BIN:$SIFTGPU_INCLUDE_DIRS:$PATH \
	LD_LIBRARY_PATH=$SIFTGPU_LIB:$LIBSVMLIN_DIR:$LUA_PATH:$LD_LIBRARY_PATH

#LIBLINEAR
RUN cd himrep && wget https://www.csie.ntu.edu.tw/~cjlin/liblinear/oldfiles/liblinear-1.91.tar.gz 
RUN cd himrep && tar xvzf liblinear-1.91.tar.gz && rm liblinear-1.91.tar.gz
RUN cd himrep/liblinear-1.91 && mkdir build && cd build && cmake .. -DCMAKE_BUILD_TYPE=RELEASE && make && cp libLiblinear.a ../
ENV LIBSVMLIN_DIR=$SRC_FOLDER/himrep/liblinear-1.91

# Himrep
ARG himrepHash=c1e4b0509da512ee4e9234d77148bf18fe228ad8
RUN cd himrep && git checkout -b localBranch $himrepHash && mkdir build && cd build && 	\
	cmake .. 	-DCMAKE_BUILD_TYPE=RELEASE 									\
				-DOpenCV_DIR=$SRC_FOLDER/opencv2/build/install/share/OpenCV \
				-DSIFTGPU_DIR=$SIFTGPU_ROOT 								\
				-DSIFTGPU_LIBRARIES=$SIFTGPU_BIN/libsiftgpu.so 				\
				-DSIFTGPU_INCLUDE_DIRS=$SIFTGPU_INCLUDE_DIRS				\
				-DCMAKE_INSTALL_PREFIX=$CMAKE_PREFIX_PATH					\
				-DCUDA_USE_STATIC_CUDA_RUNTIME=OFF 						&& 	\
	make -j$cores && make -j$cores install

#Stereo Vision
ARG stereoHash=a5b8e2599916ac1a83e7f89ac94d15d087ef3a59
RUN cd stereo-vision && git checkout -b localBranch $stereoHash && mkdir build && cd build 	&& 	\
	cmake .. 	-DCMAKE_BUILD_TYPE=RELEASE 														\
				-DUSE_SIFT_GPU=OFF 																\
				-DOpenCV_DIR=$SRC_FOLDER/opencv3/build/install/share/OpenCV 					\
				-DCUDA_USE_STATIC_CUDA_RUNTIME=OFF												\
				-DCMAKE_INSTALL_PREFIX=$CMAKE_PREFIX_PATH 									&& 	\
	make -j$cores && make -j$cores install

# Speech
ARG speechHash=0ad96ea84f490df9596544f4ca579e259783e650
RUN cd speech && git checkout -b localBranch $speechHash && mkdir build && cd build && 	\
	cmake .. 	-DCMAKE_BUILD_TYPE=RELEASE 												\
				-DCMAKE_INSTALL_PREFIX=$CMAKE_PREFIX_PATH 							&& 	\
	make -j$cores && make -j$cores install

ENV YARP_DATA_DIRS=$YARP_DIR/share/yarp:$ICUB_DIR/share/iCub:$ICUBCONTRIB_DIR/share/ICUBcontrib:$ICUBCONTRIB_DIR/share/speech:$ICUB_ROOT/app/simConfig/conf \
	YARP_PYTHONLIB=$YARP_LIB/python2.7/site-packages

RUN cd speech/svox-speech && mkdir build &&	cd build && 		\
	cmake .. 	-DCMAKE_BUILD_TYPE=RELEASE 						\
				-DCMAKE_INSTALL_PREFIX=$CMAKE_PREFIX_PATH 		\
				-DENABLE_speech=ON 							&& 	\
	make -j$cores && make -j$cores install

#Boost
ARG boostHash=fbe3e245a0b1aecfda960c1d7a254a1deeb58022
RUN cd boost-mil && git checkout -b localBranch $boostHash && mkdir build && cd build 	&& 	\
	cmake .. 	-DCMAKE_BUILD_TYPE=RELEASE 													\
				-DCMAKE_INSTALL_PREFIX=$CMAKE_PREFIX_PATH 								&& 	\
	make -j$cores && make -j$cores install

#IOL
ARG iolHash=60b608ecced5c32933449d4b67c367f16221a815
RUN cd iol && git checkout -b localBranch $iolHash && mkdir build && cd build 	&& 	\
	cmake .. 	-DCMAKE_BUILD_TYPE=RELEASE 											\
				-DCMAKE_INSTALL_PREFIX=$CMAKE_PREFIX_PATH 							\
				-DOpenCV_DIR=$SRC_FOLDER/opencv3/build/install/share/OpenCV 	&& 	\
	make -j$cores && make -j$cores install

#NITE
ARG NITED=NITE-Bin-Dev-Linux-x64-v1.5.2.21
ARG NITEZ=nite-bin-linux-x64-v1.5.2.21.tar.bz2
RUN wget http://ilab.usc.edu/packages/forall/ubuntu-12.04lts/old/nite-bin-linux-x64-v1.5.2.21.tar.bz2
RUN tar xvfj $NITEZ && cd $NITED && ./install.sh && rm $SRC_FOLDER/$NITEZ

#SensorKinect
ARG sensorKinectHash=15f1975d5e50d84ca06ff784f83f8b7836749a7b
RUN cd SensorKinect && git checkout -b localBranch $sensorKinectHash && \
	cd Platform/Linux/CreateRedist && ./RedistMaker && cd ../Redist/Sensor-Bin-Linux-x64-v5.1.2.1/ && ./install.sh

#kinectWrapper
ARG kinectHash=20113eae69e3e16e69048d7debd2d02dee591425
RUN cd kinect-wrapper && git checkout -b localBranch $kinectHash && mkdir build && cd build && 	\
	cmake .. 	-DCMAKE_BUILD_TYPE=RELEASE 														\
				-DCMAKE_INSTALL_PREFIX=$CMAKE_PREFIX_PATH 										\
				-DOpenCV_DIR=$SRC_FOLDER/opencv3/build/install/share/OpenCV 				&& 	\
	make -j$cores && make -j$cores install

USER root
RUN pip install --upgrade pip && pip install paramz && conda install ipyparallel -y && conda install 'pyqt<5' -y

ARG karmaWYSIWYDHash=e14488ffc7fcca750db54122174fe8c06fa397a0
RUN cd karmaWYSIWYD && git checkout -b localBranch $karmaWYSIWYDHash && mkdir build && cd build && 	\
	cmake .. 	-DCMAKE_BUILD_TYPE=RELEASE 															\
				-DCMAKE_INSTALL_PREFIX=$CMAKE_PREFIX_PATH 											\
				-DOpenCV_DIR=$SRC_FOLDER/opencv3/build/install/share/OpenCV 					&& 	\
	make -j$cores && make -j$cores install

# Git clone GPY and WYSIWYD
ARG GPYHash=845168af3bb5e61dafed4dd860644546e9c4666a
RUN cd GPy && git checkout -b localBranch $GPYHash && python setup.py build_ext --inplace 
ENV GPY_ROOT=$SRC_FOLDER/GPy \
	GPY_DIR=$SRC_FOLDER/GPy/build

ENV PYTHONPATH=$PYTHONPATH:$YARP_PYTHONLIB:$OPENCV_PYTHONLIB:$GPY_DIR:$GPY_ROOT

ARG GPYOPTHash=dd1438023e9d133af0c861a9a3c3fd24a5fa07bd
RUN cd GPyOpt && git checkout -b localBranch $GPYOPTHash && python setup.py develop
ENV GPY_ROOT=$SRC_FOLDER/GPyOpt \
	GPY_DIR=$SRC_FOLDER/GPyOpt/build
ENV PYTHONPATH=$PYTHONPATH:$GPYOPT_DIR:$GPYOPT_ROOT

#Install dlib
RUN wget http://dlib.net/files/dlib-19.2.tar.bz2 && tar xvfj dlib-19.2.tar.bz2 && \
	cd dlib-19.2/examples && mkdir build && cd build && \
	cmake .. -DOpenCV_DIR=$SRC_FOLDER/opencv3/build/install/share/OpenCV && make -j$cores && rm $SRC_FOLDER/dlib-19.2.tar.bz2

#Human Sensing SAM
RUN cd human-sensing-SAM && mkdir build && cd build 							&& 	\
	cmake .. -DCMAKE_BUILD_TYPE=Release												\
			-DOpenCV_DIR=/usr/local/src/robot/opencv/build/install/share/OpenCV && 	\
	make -j6 && make install

COPY CLMYarp.ini $SRC_FOLDER/human-sensing-SAM/app/CLM_Yarp/conf
ENV CLM_MODEL_DIR=$SRC_FOLDER/human-sensing-SAM/app/CLM_Yarp/conf
ENV PATH=$CLM_MODEL_DIR:$PATH

# Comment this section if postgres initialisation is not required
# COPY ABM_backup.backup .

# RUN echo "postgres:postgres" | chpasswd

# USER postgres

# RUN /etc/init.d/postgresql start &&\
#     psql --command "alter user postgres with password 'postgres'" && \
#     echo "postgres" | createdb -h localhost -p 5432 -U postgres ABM && \
#     echo "postgres" | /usr/bin/pg_restore 	--host localhost --port 5432 --username "postgres" \
# 											--dbname "ABM" --section pre-data --section data \
# 											--section post-data --verbose "/usr/local/src/robot/ABM_backup.backup"
# Comment this section if postgres initialisation is not required

USER root

#ICUB CLIENT
ARG icubClientHash=f720428148690306abc0d1b1dc1503467cc5ad02
RUN git clone https://github.com/robotology/icub-client.git
RUN cd icub-client && git checkout -b localBranch $icubClientHash && mkdir build && cd build && \
 	cmake ..	-DCMAKE_INSTALL_PREFIX=./install 													\
 				-DCMAKE_BUILD_TYPE=Release															\
 				-DOpenCV_DIR=$SRC_FOLDER/opencv3/build/install/share/OpenCV 						\
 				-DICUBCLIENT_COMPILE_TESTS=ON 													&& 	\
	make -j$cores && make -j$cores install

ENV ICUBCLIENT_ROOT=$SRC_FOLDER/icub-client/build 
ENV ICUBCLIENT_DIR=$ICUBCLIENT_ROOT/install 									\
	ICUBCLIENT_BIN=$ICUBCLIENT_ROOT/install/bin 								\
	ICUBCLIENT_LIB=$ICUBCLIENT_ROOT/install/lib 								
ENV LD_LIBRARY_PATH=$ICUBCLIENT_LIB:$LD_LIBRARY_PATH 						\
	PATH=$ICUBCLIENT_BIN:$PATH

RUN cd icub-client/bindings && mkdir build && cd build && \
	cmake .. 	-DCMAKE_INSTALL_PREFIX=./install 													\
 				-DCMAKE_BUILD_TYPE=Release															\
 				-DICUBCLIENT_USE_PYTHON_VERSION=2		&& 	\
 	make -j$cores && make -j$cores install												

ENV SAM_ROOT=$SRC_FOLDER/icub-client/src/modules \
	ICUBCLIENT_BINDINGS=$SRC_FOLDER/icub-client/bindings/build/install/lib/python2.7/site-packages
ENV PYTHONPATH=$PYTHONPATH:$SAM_ROOT:$ICUBCLIENT_BINDINGS

WORKDIR /home/icub/
COPY startup.sh .
COPY init.sh .
COPY myprompt /usr/bin/
RUN chmod +x startup.sh && chmod +x init.sh

USER icub
RUN yarp conf 10.0.0.100 10000
COPY bashrc .
COPY Xylo.wav .

RUN ssh-keygen -t rsa -f /home/icub/.ssh/id_rsa -q -N "" && touch .ssh/authorized_keys && cat .ssh/id_rsa.pub >> .ssh/authorized_keys

USER root
RUN chown -R icub /home/icub && chown -R icub $SRC_FOLDER

USER icub

# Uncomment this line if postgresql is not required
ENTRYPOINT sudo service ssh start && bash

#Comment this line if postgresql is not required
# ENTRYPOINT sudo service ssh start && sudo service postgresql start && bash

