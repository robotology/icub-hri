sudo apt-get install -y libopenni-dev squid-deb-proxy
export NITE=NITE-Bin-Dev-Linux-x64-v1.5.2.21
cd ./NITE && ./install.sh

#SensorKinect
cd SensorKinect/Platform/Linux/CreateRedist && ./RedistMaker && cd ../Redist/Sensor-Bin-Linux-x64-v5.1.2.1/ && ./install.sh

#for usb interfaces with voirtualbox
sudo usermod -aG vboxusers <username> 
sudo apt-get install virtualbox-ext-pack paprefs

#for pulseaudio sound output (ALSA required for wysiwyd)
# echo "Go to 'Network Server > Enable Network Access to other devices' to be able to get pulseaudio to work"
# paprefs 

# echo "export PULSE_SERVER=$(pax11publish | sed 's/.*tcp:/tcp:/g;s/ .*//g' | head -n1)" >> ~/.bashrc
# echo "Restart before use"