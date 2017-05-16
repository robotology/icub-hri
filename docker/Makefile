.PHONY: build first_run run clean docker build_image configure makeContainer buildBashrc

tagName := dcamilleri13/pepper-gazebo:kinetic
containerName := pepper-gazebo-kinetic
#scan parts directory
INPUTS := $(wildcard ${DOCKERPARTS}/*.part)
HOST=$(shell hostname)

build: clean build_image

build_image:
	nvidia-docker build -t $(tagName) .
	paplay ${BEEP}

run:
	nvidia-docker start $(containerName)
	nvidia-docker attach $(containerName)

clean:
	@rm -f bashrc_iCub
	-@docker rm -f $(containerName) >/dev/null || true

configure:
	sudo apt-get install -y libopenni-dev squid-deb-proxy

first_run: makeContainer buildBashrc

makeContainer:
	-@docker stop $(containerName) >/dev/null || true
	-@docker rm $(containerName) >/dev/null || true
	touch bashrc_iCub

	nvidia-docker run -it --env="DISPLAY="${DISPLAY} --env="QT_X11_NO_MITSHM=1" 					\
					  --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" 									\
					  --volume=/dev/snd:/dev/snd													\
					  --volume="/home/daniel/.local/share/yarp:/home/icub/.local/share/yarp:rw" 	\
					  --volume=$(PWD)/bashrc_iCub:/home/icub/.bashrc_iCub:rw 						\
					  --volume="/dev/bus/usb:/dev/bus/usb"											\
					  --name=${containerName} 														\
					  --hostname=$(HOST)															\
					  --network=host																\
					  --privileged 																	\
					  $(tagName)


	#set up display
	@xhost +local:`docker inspect --format='{{ .Config.Hostname }}' $(containerName)`

buildBashrc:
	#set up environment bashrc_iCub
	$(eval envVars = $(shell docker inspect --format='{{.Config.Env}}' $(containerName))) \
	$(eval envVars = $(envVars) | cut -c 31- | rev | cut -c 2- | rev)\
	$(eval envVars = $(shell echo  $(envVars)))
	@echo "export "$(envVars) > bashrc_iCub
	@echo "source ~/.sourceScripts" >> bashrc_iCub
	@echo "environment written to bashrc_iCub"

all: build first_run run
