#!/usr/bin/env python
from I3Tray import *

from icecube import icetray, dataclasses, dataio, sim_services, phys_services

from icecube.geometry_maker.detectors import *
from icecube.geometry_maker.strings import SimpleString
from icecube.geometry_maker.floors import SimpleFloor
from icecube.geometry_maker.oms import mDOM, ANTARESOM
from icecube.geometry_maker.pmts import MultiPMT

from os.path import expandvars
import math, numpy

tray = I3Tray()

tray.AddModule("I3InfiniteSource","streams",
               Stream=icetray.I3Frame.DAQ,
               Prefix=expandvars("$I3_PORTS/test-data/sim/GeoCalibDetectorStatus_IC86.55380_corrected.i3.gz"))

tray.AddModule("I3MCEventHeaderGenerator","gen_header",
               Year=2009,
               DAQTime=158100000000000000,
               RunNumber=1,
               EventID=1,
               IncrementEventID=True)

## empty GCD
#tray.AddService("I3EmptyStreamsFactory", "empty_streams",
#    InstallCalibration=True,
#    InstallGeometry=True,
#    InstallStatus=True)
#tray.AddModule("I3MetaSynth", "synth")

tray.AddModule("I3GeometryDecomposer", "decompose")

# from superpingu GCD
stringXYPositions = numpy.array([
[ 52.0759 , -29.67 ],
[ 47.8889 , -26.9353 ],
[ 42.905 , -27.3494 ],
[ 39.2267 , -30.7377 ],
[ 38.4056 , -35.6709 ],
[ 40.7881 , -40.0679 ],
[ 45.3691 , -42.0741 ],
[ 50.2162 , -40.8431 ],
[ 53.2849 , -36.8942 ],
[ 68.1533 , -29.4481 ],
[ 66.5396 , -24.7348 ],
[ 63.9365 , -20.487 ],
[ 60.4695 , -16.9093 ],
[ 56.3056 , -14.1742 ],
[ 51.6453 , -12.4132 ],
[ 46.713 , -11.7112 ],
[ 41.7464 , -12.1021 ],
[ 36.9847 , -13.567 ],
[ 32.6573 , -16.0353 ],
[ 28.9725 , -19.3882 ],
[ 26.1078 , -23.4642 ],
[ 24.2013 , -28.0669 ],
[ 23.3448 , -32.9747 ],
[ 23.5795 , -37.9511 ],
[ 24.8941 , -42.7564 ],
[ 27.2253 , -47.1593 ],
[ 30.4608 , -50.9476 ],
[ 34.4448 , -53.9388 ],
[ 38.9853 , -55.989 ],
[ 43.8637 , -56.9993 ],
[ 48.8451 , -56.921 ],
[ 53.6894 , -55.758 ],
[ 58.1633 , -53.5662 ],
[ 62.0513 , -50.4513 ],
[ 65.1662 , -46.5633 ],
[ 67.358 , -42.0894 ],
[ 68.521 , -37.2451 ],
[ 83.3688 , -29.4139 ],
[ 82.3811 , -24.5155 ],
[ 80.7542 , -19.7907 ],
[ 78.5168 , -15.3225 ],
[ 75.708 , -11.1896 ],
[ 72.3773 , -7.46438 ],
[ 68.5832 , -4.21243 ],
[ 64.3923 , -1.49084 ],
[ 59.8783 , 0.652574 ],
[ 55.1205 , 2.18015 ],
[ 50.2024 , 3.06505 ],
[ 45.2105 , 3.29173 ],
[ 40.2324 , 2.85621 ],
[ 35.3557 , 1.76614 ],
[ 30.666 , 0.0406638 ],
[ 26.2457 , -2.28989 ],
[ 22.1725 , -5.18459 ],
[ 18.5179 , -8.59257 ],
[ 15.3461 , -12.454 ],
[ 12.7129 , -16.7009 ],
[ 10.6645 , -21.2588 ],
[ 9.23687 , -26.0476 ],
[ 8.45516 , -30.9832 ],
[ 8.33307 , -35.9787 ],
[ 8.87275 , -40.9465 ],
[ 10.0647 , -45.7993 ],
[ 11.888 , -50.4519 ],
[ 14.3106 , -54.8224 ],
[ 17.29 , -58.8341 ],
[ 20.7738 , -62.4166 ],
[ 24.7007 , -65.5068 ],
[ 29.0019 , -68.0505 ],
[ 33.6017 , -70.003 ],
[ 38.4194 , -71.33 ],
[ 43.3702 , -72.0082 ],
[ 48.3672 , -72.0256 ],
[ 53.3226 , -71.382 ],
[ 58.1494 , -70.0887 ],
[ 62.7627 , -68.1683 ],
[ 67.0816 , -65.6547 ],
[ 71.03 , -62.592 ],
[ 74.5387 , -59.034 ],
[ 77.546 , -55.0431 ],
[ 79.9991 , -50.6896 ],
[ 81.8548 , -46.0499 ],
[ 83.0806 , -41.2056 ],
[ 83.655 , -36.2416 ],
[ 98.4635 , -29.413 ],
[ 97.7562 , -24.4708 ],
[ 96.5843 , -19.6177 ],
[ 94.9585 , -14.8973 ],
[ 92.8932 , -10.3519 ],
[ 90.4071 , -6.02228 ],
[ 87.5225 , -1.94739 ],
[ 84.2652 , 1.83623 ],
[ 80.6645 , 5.29465 ],
[ 76.7526 , 8.3968 ],
[ 72.5648 , 11.1149 ],
[ 68.1385 , 13.4244 ],
[ 63.5136 , 15.3048 ],
[ 58.7314 , 16.739 ],
[ 53.835 , 17.7143 ],
[ 48.8683 , 18.2219 ],
[ 43.8759 , 18.2572 ],
[ 38.9025 , 17.8199 ],
[ 33.9928 , 16.9139 ],
[ 29.1908 , 15.5474 ],
[ 24.5398 , 13.7326 ],
[ 20.0813 , 11.4859 ],
[ 15.8555 , 8.82727 ],
[ 11.9002 , 5.78072 ],
[ 8.25089 , 2.37356 ],
[ 4.94042 , -1.36364 ],
[ 1.99846 , -5.39734 ],
[ -0.548599 , -9.69134 ],
[ -2.67789 , -14.2071 ],
[ -4.37029 , -18.9041 ],
[ -5.61063 , -23.7401 ],
[ -6.38777 , -28.6719 ],
[ -6.69473 , -33.655 ],
[ -6.52877 , -38.6448 ],
[ -5.89136 , -43.5965 ],
[ -4.78824 , -48.4657 ],
[ -3.22929 , -53.2087 ],
[ -1.22852 , -57.7828 ],
[ 1.19612 , -62.1471 ],
[ 4.02287 , -66.2624 ],
[ 7.22637 , -70.0917 ],
[ 10.7778 , -73.6006 ],
[ 14.6454 , -76.7578 ],
[ 18.7944 , -79.5348 ],
[ 23.1876 , -81.9067 ],
[ 27.7855 , -83.8522 ],
[ 32.5469 , -85.3539 ],
[ 37.429 , -86.3983 ],
[ 42.3881 , -86.9761 ],
[ 47.3795 , -87.0819 ],
[ 52.3586 , -86.715 ],
[ 57.2806 , -85.8785 ],
[ 62.1014 , -84.58 ],
[ 66.7776 , -82.8312 ],
[ 71.2674 , -80.6477 ],
[ 75.5304 , -78.0491 ],
[ 79.5284 , -75.0588 ],
[ 83.2255 , -71.7035 ],
[ 86.5885 , -68.0135 ],
[ 89.5871 , -64.0218 ],
[ 92.1946 , -59.7642 ],
[ 94.3876 , -55.279 ],
[ 96.1462 , -50.6064 ],
[ 97.4548 , -45.7884 ],
[ 98.3016 , -40.8682 ],
[ 98.6789 , -35.8899 ],
[ 113.516 , -29.4064 ],
[ 112.963 , -24.44 ],
[ 112.046 , -19.5279 ],
[ 110.769 , -14.6969 ],
[ 109.14 , -9.97311 ],
[ 107.166 , -5.38244 ],
[ 104.859 , -0.949862 ],
[ 102.231 , 3.30048 ],
[ 99.2974 , 7.34543 ],
[ 96.073 , 11.1629 ],
[ 92.5759 , 14.7322 ],
[ 88.825 , 18.0339 ],
[ 84.8408 , 21.0498 ],
[ 80.645 , 23.7637 ],
[ 76.2605 , 26.1607 ],
[ 71.711 , 28.2277 ],
[ 67.0216 , 29.9536 ],
[ 62.2176 , 31.3288 ],
[ 57.3252 , 32.346 ],
[ 52.3711 , 32.9995 ],
[ 47.3824 , 33.2859 ],
[ 42.3861 , 33.2035 ],
[ 37.4094 , 32.7528 ],
[ 32.4796 , 31.9362 ],
[ 27.6235 , 30.7582 ],
[ 22.8674 , 29.2253 ],
[ 18.2374 , 27.3457 ],
[ 13.7587 , 25.1297 ],
[ 9.45556 , 22.5894 ],
[ 5.35155 , 19.7387 ],
[ 1.46899 , 16.5929 ],
[ -2.17097 , 13.1694 ],
[ -5.54848 , 9.48671 ],
[ -8.64517 , 5.56493 ],
[ -11.4441 , 1.42542 ],
[ -13.9302 , -2.90927 ],
[ -16.0897 , -7.41552 ],
[ -17.9109 , -12.0688 ],
[ -19.384 , -16.8437 ],
[ -20.5008 , -21.7143 ],
[ -21.2554 , -26.654 ],
[ -21.6435 , -31.6359 ],
[ -21.6632 , -36.6328 ],
[ -21.3142 , -41.6176 ],
[ -20.5984 , -46.563 ],
[ -19.5199 , -51.4422 ],
[ -18.0843 , -56.2286 ],
[ -16.2997 , -60.896 ],
[ -14.1756 , -65.4191 ],
[ -11.7237 , -69.7732 ],
[ -8.95736 , -73.9345 ],
[ -5.89158 , -77.8805 ],
[ -2.54309 , -81.5896 ],
[ 1.06987 , -85.0416 ],
[ 4.9276 , -88.2177 ],
[ 9.0091 , -91.1006 ],
[ 13.2921 , -93.6747 ],
[ 17.7533 , -95.9257 ],
[ 22.3685 , -97.8416 ],
[ 27.1123 , -99.4119 ],
[ 31.9591 , -100.628 ],
[ 36.8823 , -101.483 ],
[ 41.8552 , -101.973 ],
[ 46.8507 , -102.095 ],
[ 51.8416 , -101.848 ],
[ 56.8006 , -101.233 ],
[ 61.7008 , -100.254 ],
[ 66.5155 , -98.9167 ],
[ 71.2184 , -97.2277 ],
[ 75.7839 , -95.1965 ],
[ 80.1871 , -92.834 ],
[ 84.4041 , -90.1532 ],
[ 88.4119 , -87.1686 ],
[ 92.1886 , -83.8965 ],
[ 95.7136 , -80.3548 ],
[ 98.9679 , -76.5627 ],
[ 101.934 , -72.541 ],
[ 104.594 , -68.3114 ],
[ 106.936 , -63.8971 ],
[ 108.946 , -59.322 ],
[ 110.613 , -54.6112 ],
[ 111.927 , -49.7903 ],
[ 112.883 , -44.8856 ],
[ 113.474 , -39.9237 ],
[ 113.698 , -34.9317 ]
])*I3Units.m

domsPerString=401
domSpacing=1.*I3Units.m
stringLength=float(domsPerString-1)*domSpacing
depthAtZ0 = 1948.07*I3Units.m # depth @ IceCube z==0
bedrockDepth = 2810.*I3Units.m
bottomDOMDepth = depthAtZ0+500.*I3Units.m

myPMT = MultiPMT()
myOM = mDOM(pmtDescription=myPMT)
myFloor = SimpleFloor(omDescription=myOM)
myString = SimpleString(numberOfFloors=domsPerString, 
                        instrumentedStringLength=stringLength, 
                        distToFirstFloor=0., # distance up from the bedrock to the bottommost DOM
                        floorDescription=myFloor)

myDetector = XYListDetector(listOfXYTuples=stringXYPositions,
                          stringDescription=myString)

tray.AddModule("I3GeometryMaker","geo_maker",
    SubdetectorName = "MICA",
    DetectorService = myDetector,
    BedrockDepth = bedrockDepth,
    DepthAtZ0 = depthAtZ0,
    CenterDetector = False, # do not re-center the geometry
    #DetectorShiftPosX = 45.771827*I3Units.m,
    #DetectorShiftPosY = -34.411053*I3Units.m,
    DetectorShiftPosX = 0.*I3Units.m,
    DetectorShiftPosY = 0.*I3Units.m,
    DetectorShiftPosZ = depthAtZ0-bottomDOMDepth)

tray.AddModule("I3Writer", "writer",
    filename = "geometry_MICA.i3",
    Streams = [icetray.I3Frame.Geometry, icetray.I3Frame.Calibration, icetray.I3Frame.DetectorStatus])

tray.AddModule("Dump", "dump")

tray.AddModule("FrameCheck", "check",
    ensure_physics_has = ["I3Calibration", "I3Geometry", 
                          "I3DetectorStatus", "DrivingTime"])

tray.AddModule("TrashCan","trash")

tray.Execute(5)
tray.Finish()