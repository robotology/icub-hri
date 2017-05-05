export tagName=dcamilleri13/icub-client
export containerName=icub-client

nvidia-docker build -t $tagName .

nvidia-docker rm $containerName
touch bashrc_iCub

sudo service ssh stop

nvidia-docker run -it --env="DISPLAY="$DISPLAY --env="QT_X11_NO_MITSHM=1" 							\
					  --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" 									\
					  --volume=/dev/snd:/dev/snd													\
					  --volume="/home/$USER/.local/share/yarp:/home/icub/.local/share/yarp:rw" 	\
					  --volume=`pwd`/bashrc_iCub:/home/icub/.bashrc_iCub:rw 						\
					  --volume="/dev/bus/usb:/dev/bus/usb"											\
					  --name=$containerName 														\
					  --hostname=icub														\
					  --network=host 																\
					  --privileged 																	\
					  $tagName
					  
#set up display
export containerId=$(docker ps -l -q) 
echo $containerId 
xhost +local:`docker inspect --format='{{ .Config.Hostname }}' $containerId` 

#set up environment bashrc_iCub
envVars=$(docker inspect --format='{{.Config.Env}}' $containerName)
envVars=${envVars:31:-1}
envVars=$envVars | tr " " "\n"
echo "export $envVars" > bashrc_iCub
echo "source $YARP_ROOT/scripts/yarp_completion" >> bashrc_iCub

nvidia-docker start $containerId
nvidia-docker attach $containerId