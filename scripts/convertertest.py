#!/usr/bin/env python3

import os
import sys
import argparse
from astropy.io import fits
import h5py
import numpy as np
from numpy.testing import assert_equal, assert_allclose

parser = argparse.ArgumentParser(description="Test for the HDF5 converter")
parser.add_argument('filename', help='Converted HDF5 filename')
parser.add_argument('-o', '--original', help="Original FITS filename. The default is the same name as the HDF5 file with the suffix replaced.")
args = parser.parse_args()

hdf5name = args.filename
fitsname = args.original

if not fitsname:
    basefilename, _ = os.path.splitext(hdf5name)
    fitsname = basefilename + ".fits"

fitsfile = fits.open(fitsname)
hdf5file = h5py.File(hdf5name)

fitsdata = fitsfile[0].data
hdf5data = hdf5file["0/DATA"]

# CHECK MAIN DATASET

assert_equal(hdf5data, fitsdata, err_msg="Main dataset differs.")

ndim = fitsdata.ndim

if ndim == 2:
    axis_names = "XY"
elif ndim == 3:
    axis_names = "XYZ"
elif ndim == 4:
    axis_names = "XYZW"
else:
    sys.exit("Error: unsupported image dimensions.")

axes = {v: k for k, v in enumerate(reversed(axis_names))}
dims = {v: fitsdata.shape[k] for k, v in enumerate(reversed(axis_names))}

# CHECK SWIZZLES

swizzled_name = None

if ndim == 3:
    swizzled_name = "ZYX"
elif ndim == 4:
    swizzled_name = "ZYXW"

if swizzled_name:
    swizzled_shape = tuple(dims[a] for a in reversed(swizzled_name))    
    assert swizzled_name in hdf5file["0/SwizzledData"], "No swizzled dataset found."
    assert hdf5file["0/SwizzledData"][swizzled_name].shape == swizzled_shape, "Swizzled dataset has incorrect dimensions."

# CHECK STATS

stats = ["XY"]

if ndim >= 3 and dims["Z"] > 1:
    stats.append("Z")
    stats.append("XYZ")
    
for s in stats:
    assert s in hdf5file["0/Statistics"], "%s statistics missing." % s
    
    sdata = hdf5file["0/Statistics"][s]
    stats_axis = tuple(axes[a] for a in s)
    
    def assert_close(stat, func, data=fitsdata):
        if stat in sdata:
            assert_allclose(sdata[stat], func(data, axis=stats_axis), rtol=1e-5, err_msg = "%s/%s is incorrect." % (s, stat))
            
    assert_close("SUM", np.nansum)
    assert_close("SUM_SQ", np.nansum, fitsdata**2)
    assert_close("MEAN", np.nanmean)
    assert_close("MIN", np.nanmin)
    assert_close("MAX", np.nanmax)
    
    assert (np.count_nonzero(np.isnan(fitsdata), axis=stats_axis) == sdata["NAN_COUNT"]).all(), "%s/NAN_COUNT is incorrect." % s
    
# CHECK HISTOGRAMS

# TODO: check for bin size correctness
# TODO: check histogram values

fitsfile.close()
hdf5file.close()