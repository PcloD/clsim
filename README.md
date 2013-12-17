clsim
=====
An OpenCL-based photon-tracking simulation using a (backward) ray tracing algorithm
modeling scattering and absorption of light in the deep glacial ice at the South Pole
or Mediterranean sea water.

Disclaimer
----------
This project is specific to the [IceCube neutrino telescope][icecube] (and Antares/KM3NeT)
simulation and the following description contains lots of internal jargon. It requires
the [IceTray framework][icetray]. It still has some IceCube-internal dependencies, but those
might be removed in the future.

Rationale
---------

The conventional method of sampling the arrival time distribution of photons
on photo-detectors in large-volume neutrino detectors is based on large,
interpolated look-up tables. Generating such tables and finding robust interpolation
methods has proven to be highly non-trivial. Existing tools such as *ptd*,
*photonics* (recently combined with improved interpolation methods in
*photospline*) and *km3* all perform reasonable well. They all do, however,
have drawbacks, such as binning artifacts, incomplete descriptions of the detector
medium, large memory requirements and so on. All of those tools have to rely on
interpolation, requiring careful fine-tuning of parameters during the generation
of their respective photon tables. Finally, generating photon tables usually
takes a comparably long time, making such tools inconvenient for systematic
studies, where properties such as the absorption or scattering lengths need
to be changed frequently. Usually, changing any parameter requires a full
table re-generation.

The *clsim* photon tracker aims to work around these problems by tracking
each single photon generated by a given source in the detector. This very
time-consuming process can be sped up by factors of over 100 by using GPU
hardware instead of CPUs. This has first been proven by tools like *i3mcml*
and *ppc*. Instead of the vendor-specific CUDA framework used by those
tools, *clsim* uses OpenCL with the aim of being portable to other architectures,
such as AMD/ATI or even multi-core CPUs.

Overview
--------

The first step in simulating the light yield from a given I3Particle at a DOM
is to convert the particle into a series of light-emitting "steps". Each step is
assumed to have a constant speed beta=v/c, determining the Cerenkov angle and a 
constant number of photons that should be emitted over a given length.
By default, steps are created using a full *Geant4* simulation, but alternative
parameterizations can be used to speed up the process. (*Geant4* tends to get
rather slow at higher energies (E>10TeV).) The current version of *clsim* comes
with a parameterization that is compatible to *ppc*. Parameterizations can
be used for only a sub-set of particles and energies.

Once a set of steps is generated, they are uploaded to the compute device
(i.e. the GPU). The GPU runs "kernels" in parallel that are responsible for
creating photons from the steps, propagating them through ice layers and
checking for collisions with DOMs. All photons that collided are saved with
their full information (including direction, position on the DOM, wavelength,
number of scatters, properties at its point of creation, ...). Those photons
are then sent back to the host, converted to I3FrameObjects (I3Photon) and
saved in the frame. In order to keep the GPU busy, multiple frames (==events)
are simulated in parallel. The *clsim* module takes care of correctly 
re-assembling the events afterwards.

The output of the GPU simulation step is thus a list of photons at the DOM
surface. These photons still need to be converted into hits, which is done
using a dedicated module. This module finally writes a I3MCHitSeriesMap,
compatible to all existing photon simulation output.

Basic usage
-----------

For a simple simulation run, you should process your I3MCTree using *MMC*.
Then, just use the supplied "tray segment" to add all relevant services
and modules to the I3Tray:

    tray.AddSegment(clsim.I3CLSimMakeHits, "makeCLSimHits")

This reads the 'I3MCTree' and 'MMCTrackList' objects from each frame
and adds hits named 'MCHitSeriesMap'. All photons are generated in a way
compatible to *ppc*.

By default, *clsim* uses only the GPU for photon propagation (i.e.
CPU devices are skipped during OpenCL device enumeration). This behavior can
be changed using the `UseGPUs` and `UseCPUs` boolean options. The following
example would only use the CPU for simulation and skip all GPU devices:

    tray.AddSegment(clsim.I3CLSimMakeHits, "makeCLSimHits",
                    UseGPUs=False, UseCPUs=True)

Generating Photons using Geant4
-------------------------------

By default, *clsim* does apply photon yield parameterizations compatible
to *ppc*. This can, however, be turned off to use a full *Geant4* simulation
run. In this case, full GPU simulation might not be necessary because the photon
generation will become the bottleneck. 

To use *Geant4*, just enable the `UseGeant4` switch:

    tray.AddSegment(clsim.I3CLSimMakeHits, "makeCLSimHits",
                    UseGeant4=True)

Even in this mode, *Geant4* will **not** be used for muons that have a length
assigned. These are assumed to have been generated by *MMC*. Both, neutrino-generator
and Corsika generate muons without lengths, so the module should generally
do the right thing. To make absolutely sure that no parameterizations are used,
also set the `MMCTrackListName` option to `None`. This will apply *Geant4* to all
particles in the I3MCTree (which are `InIce` and not `Dark`), even if they
already have a length assigned to them. (You should make sure that this is what you
really want, as MMC might already have added cascades to the muon track that would
be added a second time by Geant4.)

Low-Energy simulations
----------------------

In order to simulate low energies in a more correct way, you might want to
consider disabling the DOM oversizing optimization. It is set to an oversize
factor of 5 (in radius), which gives you a 25-fold increase in simulation
speed at the expense of accuracy in timing and for tracks very close to DOMs.

To disable DOM oversizing (which might be a good idea especially when using Geant4)
use the DOMOversizeFactor switch:

    tray.AddSegment(clsim.I3CLSimMakeHits, "makeCLSimHits",
                    DOMOversizeFactor=1., # disables oversizing (default is 5.)
                    UseGeant4=True)       # enable or disable Geant4 as needed

Ice Models
----------

By default, the 'SPICE-Mie' ice model is used. This can, however, easily be changed by
supplying either a *photonics*-compatible ice description file or a
*ppc*-compatible ice description directory using the 'IceModelLocation'
parameter:

    tray.AddSegment(clsim.I3CLSimMakeHits, "makeCLSimHits",
                    IceModelLocation=expandvars("$I3_SRC/clsim/resources/ice/ppc_aha_0.80"))

Another example (using a *photonics* file) would be:

    tray.AddSegment(clsim.I3CLSimMakeHits, "makeCLSimHits",
                    IceModelLocation=expandvars("$I3_SRC/clsim/resources/ice/photonics_wham/Ice_table.wham.i3coords.cos090.11jul2011.txt"))


License
-------
Except the files listed below, all of the code is under the [ISC (OpenBSD/BSD 2 Clause equivalent) license][license].

The files `clsim/private/geant4/TrkCerenkov.cxx` and `clsim/private/geant4/TrkCerenkov.hh` are based on code from the Geant4 package released under the [Geant4 license][geant4license] ( http://cern.ch/geant4/license ).

This product includes software developed by Members of the [Geant4 Collaboration][geant4coll] ( http://cern.ch/geant4 ).

[icecube]: http://icecube.wisc.edu
[icetray]: http://code.icecube.wisc.edu/projects/icetray
[license]: https://github.com/claudiok/clsim/master/resources/docs/LICENSE
[geant4license]: http://cern.ch/geant4/license
[geant4coll]: http://cern.ch/geant4