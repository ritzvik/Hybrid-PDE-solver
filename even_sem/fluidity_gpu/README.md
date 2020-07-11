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

## Installing fluidity.

We would not install fluidity inside the image so as to maintain flexiblity and scope for tests. Instead, we would build fluidity ouside the container using the environment of the container.

```
# disable exclusive root access to built images
sudo chmod 777 *simg

# shallow-clone the fluidity-repo
git clone --depth 2 https://github.com/FluidityProject/fluidity.git
```

Folder starting with `scratch*/` are ignored as it is mentioned in `.gitignore`. Thus any test folders that needs to be made, acn start with `scratch*/`

```
# rename the cloned fluidity folder due to the reasons mentioned above

mv fluidity/ scratch0/

# make another copy of it
cp -r scratch0/ scratch1/
```

Leave `scratch0` unchanged while do the GPU-specific changes in the source code of fluidity in `scratch1` as
mentioned in the [report](../reports/final_report.pdf), section `2.2.1`.
