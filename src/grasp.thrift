# Copyright: (C) 2020 iCub Fondazione Istituto Italiano di Tecnologia (IIT) 
# All Rights Reserved.
#
# grasp.thrift

/**
* grasp_IDL
*
* IDL Interface
*/
service grasp_IDL
{
   /**
   * Go through the whole sequence.
   * @return true/false on success/failure.
   */
   bool grasp();
}
