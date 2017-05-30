#ifndef HOMEO_MANAGER
#define HOMEO_MANAGER

#include <string>
#include <iostream>
#include <iomanip>
#include <vector>
#include "homeostasis.h"

class HomeostasisManager
{

public:

    std::vector<Drive*> drives;

    /**
     * @brief  addDrive Generates a drive in the homeostasis manager. 
        Homeostasis manager will take care of updating it. 
        Input must be a drive and its position
     * @param D drive instance to add
     * @return VOID
     */
    void addDrive(Drive* D);

    /**
     * @brief removeDrive Removes a drive in homeostasis manager. 
        Anything related to this drive outside here should also be 
        removed. This is specially useful for temporal 
        needs or subgoals.
     * @param  D drive ID to remove
     * @return VOID
     */
    void removeDrive(int D);

    /**
     * @brief sleep cancels update function for a time
     * @param  D ID drive to add
     * @param  time time to sleep
     * @return VOID
     */
    void sleep(int D, double time);

    /**
     * @brief updateDrives updates all internal values
     * @param  none
     * @return bool success
     */
    bool updateDrives();
    

};

#endif
