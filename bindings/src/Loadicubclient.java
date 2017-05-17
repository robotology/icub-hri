/*
 * Copyright: (C) 2010 RobotCub Consortium
 * Author: Lorenzo Natale, Paul Fitzpatrick
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 */

// run this to load the iCubclient library

class LoadIcubclient {
    private static int icubclientInitialized = 0;

    public static int isLoaded() { return icubclientInitialized; }

    public LoadIcubclient() {
        if (icubclientInitialized == 0) {
            System.loadLibrary("jicubclient");
            Network.init();
            System.out.println("iCubClient library loaded and initialized");
            System.out.flush();
        } else {
            System.out.println(
                "iCubClient library already loaded and initialized, doing nothing");
            System.out.flush();
        }
        icubclientInitialized = 1;
    }
}
