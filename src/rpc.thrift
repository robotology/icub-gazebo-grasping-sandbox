# Copyright: (C) 2020 iCub Fondazione Istituto Italiano di Tecnologia (IIT) 
# All Rights Reserved.
#
# rpc.thrift

/**
* rpc_IDL
*
* IDL Interface
*/
service rpc_IDL
{
   /**
   * Randomize object location.
   * @return true/false on success/failure.
   */
   bool randomize();

   /**
   * Go home with arms and gaze.
   * @return true/false on success/failure.
   */
   bool home();

   /**
   * Segment out the object point cloud.
   * @return true/false on success/failure.
   */
   bool segment();

   /**
   * Fit the object point cloud with a superquadric.
   * @return true/false on success/failure.
   */
   bool fit();

   /**
   * Grasp the object.
   * @return true/false on success/failure.
   */
   bool grasp();

   /**
   * Go the whole hog.
   * @param random_pose iff "on" it performs object's pose randomization
   * @return true/false on success/failure.
   */
   bool go(1:string random_pose);
}
