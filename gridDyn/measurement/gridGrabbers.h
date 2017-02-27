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

#ifndef GRID_GRABBER_H_
#define GRID_GRABBER_H_

#include "core/objectOperatorInterface.h"
#include "units.h"
#include "gridDynDefinitions.h"
#include <functional>
#include <memory>

class gridCore;


//TODO:: PT make more of the parameters protected
/* class that grabs data from another object directly
@details there is a target object and a functional object that actually can extract data from that object
it also includes a gain and bias to do a linear shift and scale on the object
*/
class gridGrabber: public objectOperatorInterface
{
public:
  std::string field; //!< the target field that is being grabbed
  //TODO:: convert to a bitset
  bool loaded = false;	//!< flag if the grabber is loaded
  bool vectorGrab = false;  //!< flag if the grabber is meant to grab a vector of data
  bool useVoltage = false; //!< flag indicating use of the baseVoltage for unit conversion
  bool cloneable = true;  //!< flag indicating if the grabber is cloneable
  bool customDesc = false;	//!< flag indicating the grabber uses a custom description
  gridUnits::units_t outputUnits = gridUnits::defUnit; //!< the desired input units of the grabber
  gridUnits::units_t inputUnits = gridUnits::defUnit;  //!< the units of the actual grabbed data
  double gain = 1.0;	//!< gain multiplier on the output data
  double bias = 0.0;	//!< bias shift on the output
  double m_baseVoltage = 100; //!< the base voltage on the grabber if needed
protected:
  mutable std::string desc;  //!< a description of the grabber
  coreObject *cobj = nullptr; //!< the target core object to grab the data from
  std::function<double (coreObject *)> fptr; //!< operation function to grab a single data element
  std::function<void(coreObject *, std::vector<double> &)> fptrV; //!< operation function to grab a vector of data
  std::function<void(coreObject *, stringVec &)> fptrN;  //!< function to grab a vector of strings corresponding to the vector of data
public:
	gridGrabber(const std::string &fld = "");

	gridGrabber(const std::string &fld, coreObject *obj);

  virtual ~gridGrabber ()
  {
  }
	/** clone function
	 *@param[in] ggb a pointer to another gridGrabber function if we are cloning on existing object
	 *@return a shared_ptr to another GridGrabber*/
  virtual std::shared_ptr<gridGrabber> clone (std::shared_ptr<gridGrabber> ggb = nullptr) const;
	/** update the field of grabber
	 *@param[in]  fld the new field to capture
	 *@throw unrecognized parameter exception*/
  virtual void updateField (const std::string &fld);
	/** actually go and get the data
	 *@return the value produced by the grabber*/
  virtual double grabData ();
	/** @brief grab a vector of data
	 *@param[out] data the vector to store the data in
	 */
  virtual void grabData (std::vector<double> &data);
	/** @brief get the descriptions of the data
	 *@param[out] desc_list  the list of descriptions
	 **/
  virtual coreTime getTime() const;
  virtual void getDesc (std::vector<std::string > &desc_list) const;

  virtual const std::string &getDesc() const;
  void setDescription(const std::string &newDesc)
  {
	  desc = newDesc;
	  customDesc = true;
  }
  virtual void updateObject (coreObject *obj, object_update_mode mode = object_update_mode::direct) override;
  virtual coreObject * getObject() const override;
  virtual void getObjects(std::vector<coreObject *> &objects) const override;
protected:
	/** @brief generate the description(s)*/
  virtual void makeDescription () const;
	/** @brief check if the grabber is ready to actually grab data
	 * @return true if it is ready, false otherwise
	 */
  virtual bool checkIfLoaded();
};

/** custom grabber function class
*/
class customGrabber : public gridGrabber
{
public:
  void setGrabberFunction (std::string fld,std::function<double (coreObject *)> nfptr);
  void setGrabberFunction(std::function<void(coreObject *, std::vector<double> &)> nVptr);
protected:
	virtual bool checkIfLoaded() override;
};

std::unique_ptr<gridGrabber> createGrabber (const std::string &fld, coreObject *obj);
std::unique_ptr<gridGrabber> createGrabber (int noffset, coreObject *obj);

std::vector < std::unique_ptr < gridGrabber >> makeGrabbers (const std::string & command, coreObject * obj);

/** class defining a function operator on grabber*/
class functionGrabber : public gridGrabber
{

protected:
  std::shared_ptr<gridGrabber> bgrabber; //!< the underlying grabber to get the data
  std::string function_name;  //!< the name of the function
  std::function<double(double val)> opptr; //!< the function operation on the data
  std::function<double(std::vector<double>)> opptrV; //!< the function to call to get a vector of data
  std::vector<double> tempArray; //!< temporary array data location

public:
  functionGrabber ();
  functionGrabber (std::shared_ptr<gridGrabber> ggb, std::string func);
  virtual std::shared_ptr<gridGrabber> clone (std::shared_ptr<gridGrabber> ggb = nullptr) const override;
  virtual double grabData () override;
  virtual void grabData (std::vector<double> &data) override;
 virtual void updateObject (coreObject *obj, object_update_mode mode = object_update_mode::direct) override;
  virtual coreObject * getObject () const override;
  virtual void getObjects(std::vector<coreObject *> &objects) const override;
  virtual void updateField (const std::string &fld) override;
  using gridGrabber::getDesc;
  virtual void getDesc (std::vector<std::string > &desc_list) const override;
  virtual coreTime getTime() const override;
protected:
	virtual bool checkIfLoaded() override;
};

/** class implementing a operation on two grabbers */
class opGrabber : public gridGrabber
{
protected:
  std::shared_ptr<gridGrabber> bgrabber1;  //!< grabber 1 
  std::shared_ptr<gridGrabber> bgrabber2; //!< grabber 2
  std::string op_name;		//!< the name of the 2 argument operation
  std::function<double(double, double)> opptr;  //!< the function pointer to the operation
  std::function<double(std::vector<double>, std::vector<double>)> opptrV;  //!< function pointer to a vector grab operation
  std::vector<double> tempArray1,tempArray2; //!< temporary arrays for processing the data
public:
  opGrabber ();
  opGrabber (std::shared_ptr<gridGrabber> ggb1, std::shared_ptr<gridGrabber> ggb2, std::string op);
  virtual std::shared_ptr<gridGrabber> clone (std::shared_ptr<gridGrabber> ggb = nullptr) const override;
  virtual double grabData () override;
  virtual void grabData (std::vector<double> &data) override;
  virtual void updateObject (coreObject *obj, object_update_mode mode = object_update_mode::direct) override;
	/** update a specific object 
	 *@param[in] obj  the new object
	 *@param[in] num  1 for updating bgrabber 1 2 for bgrabber 2
	 *@return 0 if successful, error code otherwise
	 */
  virtual void updateObject (coreObject *obj, int num);
  virtual coreObject * getObject () const override;
  virtual void getObjects(std::vector<coreObject *> &objects) const override;
  virtual void updateField (const std::string &fld) override;
  using gridGrabber::getDesc;
  
  virtual void getDesc (std::vector<std::string > &desc_list) const override;
  virtual coreTime getTime() const override;
  
protected:
	virtual bool checkIfLoaded() override;
};

#endif