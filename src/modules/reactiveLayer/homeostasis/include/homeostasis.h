#ifndef HOMEOSTASIS_H
#define HOMEOSTASIS_H

#include <string>
#include <iostream>
#include <iomanip>
#include <math.h>
#include <limits>
#include <time.h>

/**
 * \ingroup homeostasis
 */
class Drive
{
public:
    std::string name; //!< Name of the drive
    std::string key; //!< Deprecated, not used anymore
    double period;
    double value, homeostasisMin, homeostasisMax, decay, valueMin, valueMax, default_value, decay_multiplier;
    bool gradient;
    time_t start_sleep; //!< Time when the drive started to be sleeping
    bool is_sleeping; //!< Whether drive is sleeping
    double time_to_sleep; //!< Timespan to be sleeping

    Drive(std::string d_name, double d_period, std::string d_key="default", double d_value=0.5, double d_homeo_min=0.25, double d_homeo_max=0.75, double d_decay = 0.05, double d_value_min=std::numeric_limits<double>::min(), double d_value_max=std::numeric_limits<double>::max(), bool d_gradient = false) :
        name(d_name),
        key(d_key),
        period(d_period),
        value(d_value),
        homeostasisMin(d_homeo_min),
        homeostasisMax(d_homeo_max),
        decay(d_decay),
        valueMin(d_value_min),
        valueMax(d_value_max),
        default_value((d_homeo_max + d_homeo_min)/2.),
        decay_multiplier(1),
        gradient(d_gradient),
        start_sleep(time(NULL)),
        is_sleeping(true),
        time_to_sleep(1e10)
    {
        //todo : check the min/max
        if (d_value_min == std::numeric_limits<double>::min() && d_value_max == std::numeric_limits<double>::max()){
            double homeoRange =  homeostasisMax - homeostasisMin;
            valueMin = homeostasisMin - 0.5 * homeoRange;
            valueMax = homeostasisMax + 0.5 * homeoRange;
        }
    }

    Drive() = delete; //!< No default constructor!

    void setKey(std::string d_key)
    {
        this->key=d_key;
    }
    void setValue(double d_value)
    {
        this->value=d_value;
    }
    void deltaValue(double d_value)
    {
        this->value += d_value;
    }

    void setHomeostasisMin(double d_homeo_min)
    {
        this->homeostasisMin = d_homeo_min;
    }
    void deltaHomeostasisMin(double d_homeo_min)
    {
        this->homeostasisMin += d_homeo_min;
    }

    void setHomeostasisMax(double d_homeo_max)
    {
        this->homeostasisMax = d_homeo_max;
    }
    void deltaHomeostasisMax(double d_homeo_max)
    {
        this->homeostasisMax += d_homeo_max;
    }
   
    void setDecay(double d_decay)
    {
        this->decay = d_decay;
    }
    void deltaDecay(double d_decay)
    {
        this->decay += d_decay;
    }

    void setDecayMultiplier(double mult)
    {
        this->decay_multiplier = mult;
    }

    /**
     * @brief sleep Sleep drive for t seconds, i.e. drive won't decay
     * @param t time to sleep
     */
    void sleep(double t) {
        start_sleep = time(NULL);
        is_sleeping = true;
        time_to_sleep = t;
    }

    /**
     * @brief freeze Freeze (sleep) drive indefinitely
     */
    void freeze() {
        start_sleep = time(NULL);
        is_sleeping = true;
        time_to_sleep = 1e10;
    }

    /**
     * @brief reset Set value back to default_value
     */
    void reset(){
        value = default_value;
    }

    /**
     * @brief unfreeze Unfreeze drive
     */
    void unfreeze() {
        is_sleeping = false;
    }

    /**
     * @brief update Let drive decay
     */
    void update()
    {
        if (is_sleeping) {
            if (difftime(time(NULL), start_sleep) > time_to_sleep) {
                is_sleeping = false;
            }
        }
        else if (! ((this->value > valueMax && this->decay<0) || (this->value < valueMin && this->decay>0))) {
            this->value -= (this->decay * this->decay_multiplier * period);          
        }
    }

    double getValue()
    {
        return(value);
    }

    void setName(std::string n)
    {
        this->name = n;
    }

    void setGradient(bool b)
    {
        this->gradient = b;
    }

};

#endif
