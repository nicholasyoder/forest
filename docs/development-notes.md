# Development Notes

## Build deb package

Note: replace `0.7.8` with the correct version number of the release.

1. Modify `debian/changelog`
2. Copy / rename `forest` to a build directory named `forest-0.7.8`
3. `cd forest-0.7.8`
4. `dh_make -e <email address> -c lgpl3 --createorig`
5. `debuild`

## Include deb in repo

`reprepro includedeb forest /path/to/forest_0.7.8-1_amd64.deb`