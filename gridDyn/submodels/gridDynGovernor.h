/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
 * LLNS Copyright Start
 * Copyright (c) 2016, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
*/

#ifndef GRIDDYNGOVERNOR_H_
#define GRIDDYNGOVERNOR_H_

#include "gridObjects.h"

#include "submodels/gridControlBlocks.h"

const int govOmegaInLocation = 0;
const int govpSetInLocation = 1;
/** @brief class defining the interface for a governor
 the governor class is a really basic governor it includes two time constants
and takes as input the frequency and power setting*/
class gridDynGovernor : public gridSubModel
{
public:
  /** @brief flags for governor control*/
  enum governor_flags
  {
    ignore_deadband = object_flag2, //!< indicator that the deadband block should be ignored
    ignore_filter = object_flag3, //!< indicator that the filter block should be ignored
    ignore_throttle = object_flag4,  //!< indicator that the delay block should be ignored
    p_limited = object_flag5,       //!< indicator that power level was limited
    p_limit_high = object_flag6,   //!< indicator that the throttle level was at the high limit
    uses_plimits = object_flag7,        //!< indicator that the governor uses limits
    uses_ramplimits = object_flag8,             //!< indicator that the governor had ramp limits
  };
protected:
  double K = 16.667;             //!< [pu] droop gain (1/R)
  double T1 = 0.1;            //!< [s]   droop control time constant 1
  double T2 = 0.0;                        //!< [s]   droop control  time constant 2
  double T3 = 0.0;                              //!< [s]  throttle response
  double Pmax = kBigNum;        //!< [pu] maximum turbine output
  double Pmin = -kBigNum;        //!< [pu] minimum turbine output
  double Pset = 0.0;          //!< [pu] Set point and initial Pm
  double Wref = -kBigNum;       //!<[rad]  reference frequency
  double deadbandHigh = -kBigNum;      //!<upper threshold on the deadband;
  double deadbandLow = kBigNum;      //!<lower threshold on the deadband;
  double machineBasePower = 100.0;       //!<the machine base of the generator;
  deadbandBlock dbb;                    //!< block managing the deadband
  controlBlock cb;                      //!< block managing the filtering functions on the frequency response
  delayBlock delay;                     //!< block managing the throttle filter
public:
  /** @brief constructor*/
  gridDynGovernor (const std::string &objName = "gov_#");
  virtual gridCoreObject * clone (gridCoreObject *obj = nullptr) const override;
  /** @brief destructor*/
  virtual ~gridDynGovernor ();
  virtual void objectInitializeA (double time, unsigned long flags) override;
  virtual void objectInitializeB (const IOdata &args, const IOdata &outputSet, IOdata &inputSet) override;

  virtual int set (const std::string &param,  const std::string &val) override;
  virtual int set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit) override;
  virtual int setFlag (const std::string &flag, bool val) override;
  virtual double get (const std::string &param, gridUnits::units_t unitType = gridUnits::defUnit) const override;
  virtual index_t findIndex (const std::string &field, const solverMode &sMode) const override;
  virtual void residual (const IOdata &args, const stateData *sD, double resid[], const solverMode &sMode) override;
  virtual void derivative (const IOdata &args, const stateData *sD, double deriv[], const solverMode &sMode) override;
  virtual void jacobianElements (const IOdata &args, const stateData *sD,
                                 arrayData<double> *ad,
                                 const IOlocs &argLocs, const solverMode &sMode) override;
  virtual double timestep  (double ttime, const IOdata &args, const solverMode &sMode) override;

  virtual void rootTest (const IOdata &args, const stateData *sD, double roots[], const solverMode &sMode) override;

protected:
  //virtual void setTime(double time){prevTime=time;};
};

#endif //GRIDGOVERNOR_H_
