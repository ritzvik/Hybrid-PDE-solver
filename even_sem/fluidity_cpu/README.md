This section has instructions to get a fluidity environment running inside singulairty.

## Running the singulairty script:
  - `sudo singulairty build fluidity.simg fluidity_cpu.singularity` if no further changes are desred in the built image
  - `sudo singulairty build --writable fluidity.simg fluidity_cpu.singularity` if you need to test out specific codes inside the image. This one is recommended.

## Notes :
  - This script was made by compiling varius CI scripts from fluidity repo
  - The prerequisites of installing fluidity is met by running `apt install fluidity-dev`. This is given in the script.
  - After installing the prerequisites, final fluidity is built from source.
  - Tested on fluidity at commit number `8b7bbd85103a2da2015037dec61253cb3e7cefc0`.
  - Running fluidity codes would be discussed elaborately in [fluidity-GPU README](../fluidity_gpu/README.md).
