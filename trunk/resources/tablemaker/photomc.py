
from optparse import OptionParser
from icecube.icetray import I3Units
from os import path, unlink

usage = "usage: %prog [options] outputfile"
parser = OptionParser(usage)

parser.add_option("--seed", dest="seed", type="int", default=None,
    help="Seed for random number generators; harvested from /dev/random if unspecified.")
parser.add_option("--nevents", dest="nevents", type="int", default=100,
    help="Number of light sources to inject [%default]")
parser.add_option("--z", dest="z", type="float", default=0.,
    help="IceCube z-coordinate of light source, in meters [%default]")
parser.add_option("--zenith", dest="zenith", type="float", default=0.,
    help="Zenith angle of source, in IceCube convention and degrees [%default]")
parser.add_option("--energy", dest="energy", type="float", default=1,
    help="Energy of light source, in GeV [%default]")
parser.add_option("--step", dest="steplength", type="float", default=1,
    help="Sampling step length in meters [%default]")
parser.add_option("--overwrite", dest="overwrite", action="store_true", default=False,
    help="Overwrite output file if it already exists")
    
opts, args = parser.parse_args()

if len(args) != 1:
	parser.error("You must specify an output file!")
outfile = args[0]
if path.exists(outfile):
	if opts.overwrite:
		unlink(outfile)
	else:
		parser.error("Output file exists! Pass --overwrite to overwrite it.")

from icecube import icetray
from icecube.clsim.tablemaker.tabulator import PhotonGenerator, I3TabulatorModule, generate_seed

outfile = args[0]
if opts.seed is None:
	opts.seed = generate_seed()
opts.zenith *= I3Units.degree

from I3Tray import I3Tray

tray = I3Tray()

rng, header = tray.AddSegment(PhotonGenerator, 'generator', Seed=opts.seed,
    Zenith=opts.zenith, ZCoordinate=opts.z, Energy=opts.energy, NEvents=opts.nevents)
    
tray.AddModule(I3TabulatorModule, 'beancounter',
    Source='Source', Photons='PropagatedPhotons', Statistics='I3CLSimStatistics',
    Filename=outfile, StepLength=opts.steplength, RandomService=rng,
    TableHeader=header)
    
tray.AddModule('TrashCan', 'MemoryHole')
tray.Execute()
tray.Finish()
