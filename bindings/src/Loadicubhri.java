/*
 * Copyright: (C) 2010 RobotCub Consortium
 * Author: Lorenzo Natale, Paul Fitzpatrick
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 */

// run this to load the iCub-HRI library

class LoadIcubhri {
    private static int icubhriInitialized = 0;

    public static int isLoaded() { return icubhriInitialized; }

    public LoadIcubhri() {
        if (icubhriInitialized == 0) {
            System.loadLibrary("jicubhri");
            Network.init();
            System.out.println("iCub-HRI library loaded and initialized");
            System.out.flush();
        } else {
            System.out.println(
                "iCub-HRI library already loaded and initialized, doing nothing");
            System.out.flush();
        }
        icubhriInitialized = 1;
    }
}
