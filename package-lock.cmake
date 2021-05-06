# CPM Package Lock
# This file should be committed to version control

# dr_libs
CPMDeclarePackage(dr_libs
  NAME dr_libs
  GIT_TAG 343aa92
  DOWNLOAD_ONLY YES
  GITHUB_REPOSITORY mackron/dr_libs
)
# miniaudio
CPMDeclarePackage(miniaudio
  NAME miniaudio
  GIT_TAG 199d6a7
  DOWNLOAD_ONLY YES
  GITHUB_REPOSITORY mackron/miniaudio
)
# utfcpp (unversioned)
# CPMDeclarePackage(utfcpp
#  NAME utfcpp
#  GIT_TAG ddd38b3
#  DOWNLOAD_ONLY YES
#  GITHUB_REPOSITORY nemtrif/utfcpp
#)
# wavpack
CPMDeclarePackage(wavpack
  NAME wavpack
  GIT_TAG 7349945
  GITHUB_REPOSITORY colugomusic/wavpack
)
