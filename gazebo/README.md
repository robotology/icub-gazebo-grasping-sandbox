Scriptable models
=================

All models also contain a scriptable SDF with extension `.rsdf`. This allows deciding the mass of the object and thus generating the inertia matrix accordingly.

To generate the `sdf` from the `rsdf`, run `erb model.rsdf > model.sdf`.

The command `erb` is available once you install `ruby` on your system.
