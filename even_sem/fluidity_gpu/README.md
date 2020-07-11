Here, we would discuss installing fluidity on GPU.

## Building the singularity images
The scripts are divided into two parts, where the second one depends on the first to build.
We recommned building the second one in writable mode, so that user could carry out experiments.
Feel free to build images here only as `*img` and `*simg` extensions are ignored in `.gitignore`.

#### First build
```
sudo singulaity build fluidity_b0.simg fluidity_b0.singularity
```
Please take care that the names of the image remains as it is, as the second one depends on it.

#### Second build
`sudo singulaity build --writable fluidity_b1.simg fluidity_b1.singularity`

## Obtaining fluidity.

We would not install fluidity inside the image so as to maintain flexiblity and scope for tests. Instead, we would build fluidity ouside the container using the environment of the container.

```
# disable exclusive root access to built images
sudo chmod 777 *simg

# shallow-clone the fluidity-repo
git clone --depth 2 https://github.com/FluidityProject/fluidity.git
```

Again, mind that the fluidity commit version is `8b7bbd85103a2da2015037dec61253cb3e7cefc0` while writing this manual.

Folder starting with `scratch*/` are ignored as it is mentioned in `.gitignore`. Thus any test folders that needs to be made, acn start with `scratch*/`

```
# rename the cloned fluidity folder due to the reasons mentioned above

mv fluidity/ scratch0/

# make another copy of it
cp -r scratch0/ scratch1/
```

Leave `scratch0` unchanged while do the GPU-specific changes in the source code of fluidity in `scratch1` as
mentioned in the [report](../reports/final_report.pdf), section `2.2.1`.

## Installing Fluidity
Procedure to install fluidity from `scratch0` would be shown. Very similar steps would be applicable
for `scratch1` also.

```
# open singularity environment in shell mode
# --nv option links the GPU driver of the system isnide the image

singularity shell --nv fluidity_b1.simg
```

Now we are inside a running container, with the complete environment for installing fluidity already set-up.

```
cd scratch0
# configure fluidity
./configure --enable-2d-adaptivity
# make fluidity
make -j
make makefiles
make -j fltools
make -j all
```
```
# run tests (purely optional)
make THREADS=8 test
```

## Running a test code
Make sure that user is still in the singularity shell and inside the `scratch0` directory.
```
# cd to tests/ (all tests are here)
cd tests/
# run one of the tests
cd 3material-droplet/
make
../../tools/testharness.py --file ../../tools/testharness.py --file 3material-droplet.xml

# run it with standalone fluidity binary
../../bin/fluidity -v2 -l 3material-droplet.flml

# to time to program just append time in front of the command
time ../../bin/fluidity -v2 -l 3material-droplet.flml

```
