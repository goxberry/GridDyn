/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
* LLNS Copyright Start
* Copyright (c) 2017, Lawrence Livermore National Security
* This work was performed under the auspices of the U.S. Department
* of Energy by Lawrence Livermore National Laboratory in part under
* Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
* Produced at the Lawrence Livermore National Laboratory.
* All rights reserved.
* For details, see the LICENSE file.
* LLNS Copyright End
*/

#ifndef FMI_MESUBMODEL_H_
#define FMI_MESUBMODEL_H_

#include "gridObjects.h"
#include <map>

class fmi2ModelExchangeObject;
class outputEstimator; 


enum class fmuMode;  //forward declare enumeration

#include "fmiSupport.h"

/** class defining a subModel interacting with an FMU v2.0 object for model exchange*/
class fmiMESubModel : public gridSubModel
{
public:
	enum fmiSubModelFlags
	{
		use_output_estimator=object_flag2,
		fixed_output_interval=object_flag3,
		reprobe_flag=object_flag4,
		has_derivative_function =object_flag5,
	};
protected:
	count_t m_stateSize = 0;
	count_t m_jacElements = 0;
	count_t m_eventCount = 0;
	std::shared_ptr<fmi2ModelExchangeObject> me;

  std::vector<outputEstimator *> oEst;  //!<vector of objects used for output estimation //TODO:: Make this an actual vector of objects
  coreTime localIntegrationTime = 0.01;
  fmuMode prevFmiState;
  std::vector<vInfo> stateInformation;
  std::vector<vInfo> outputInformation;
  std::vector<int> inputVarIndices;
private:

	count_t lastSeqID = 0;
	std::vector<double> tempState;
	std::vector<double> tempdState;
  public:
	  fmiMESubModel(const std::string &newName="fmisubmodel2_#", std::shared_ptr<fmi2ModelExchangeObject> fmi=nullptr);

	  fmiMESubModel(std::shared_ptr<fmi2ModelExchangeObject> fmi = nullptr);
  virtual ~fmiMESubModel();
  virtual coreObject * clone(coreObject *obj = nullptr) const override;
protected:
  virtual void dynObjectInitializeA (coreTime time, unsigned long flags) override;
  virtual void dynObjectInitializeB (const IOdata &inputs, const IOdata &desiredOutput, IOdata &inputSet) override;
public:
  virtual void getParameterStrings(stringVec &pstr, paramStringType pstype) const override;
  virtual stringVec getOutputNames() const;
  virtual stringVec getInputNames() const;
  virtual void set (const std::string &param, const std::string &val) override;
  virtual void set (const std::string &param, double val, gridUnits::units_t unitType = gridUnits::defUnit)  override;

  virtual double get(const std::string &param , gridUnits::units_t unitType = gridUnits::defUnit) const  override;
  virtual index_t findIndex(const std::string &field, const solverMode &sMode) const  override;
  virtual void loadSizes(const solverMode &sMode, bool dynOnly) override;
  virtual void residual(const IOdata &inputs, const stateData &sD, double resid[], const solverMode &sMode) override;
  virtual void derivative(const IOdata &inputs, const stateData &sD, double deriv[], const solverMode &sMode) override;
  virtual void jacobianElements(const IOdata &inputs, const stateData &sD,
    matrixData<double> &ad,
    const IOlocs &inputLocs, const solverMode &sMode) override;
  virtual void timestep (coreTime ttime, const IOdata &inputs, const solverMode &sMode) override;
  virtual void ioPartialDerivatives(const IOdata &inputs, const stateData &sD, matrixData<double> &ad, const IOlocs &inputLocs, const solverMode &sMode) override;
  virtual void outputPartialDerivatives (const IOdata &inputs, const stateData &sD, matrixData<double> &ad, const solverMode &sMode) override;
  virtual void rootTest(const IOdata &inputs, const stateData &sD, double roots[], const solverMode &sMode) override;
  virtual void rootTrigger(coreTime ttime, const IOdata &inputs, const std::vector<int> &rootMask, const solverMode &sMode) override;

  IOdata getOutputs(const IOdata &inputs, const stateData &sD, const solverMode &sMode) const override;
  virtual double getDoutdt(const IOdata & inputs, const stateData &sD, const solverMode &sMode, index_t num = 0) const override;
  virtual double getOutput(const IOdata &inputs, const stateData &sD, const solverMode &sMode, index_t num = 0) const override;

  virtual double getOutput(index_t num = 0) const override;
  virtual index_t getOutputLoc( const solverMode &sMode, index_t num = 0) const override;


  virtual void setState(coreTime ttime, const double state[], const double dstate_dt[], const solverMode &sMode) override;
  //for saving the state
  virtual void guess(coreTime ttime, double state[], double dstate_dt[], const solverMode &sMode) override;

  virtual void getTols(double tols[], const solverMode &sMode) override;

  virtual void getStateName(stringVec &stNames, const solverMode &sMode, const std::string &prefix = "") const override;

  virtual bool isLoaded() const;

  virtual void updateLocalCache(const IOdata &inputs, const stateData &sD, const solverMode &sMode) override;
 protected:
  
  void makeSettableState();
  void resetState();
  double getPartial(int depIndex, int refIndex, refMode_t mode);
  void probeFMU();
  void loadOutputJac(int index = -1);
 // int searchByRef(fmi2_value_reference_t ref);
};


#endif