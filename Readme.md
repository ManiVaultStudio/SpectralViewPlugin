
# Spectral Viewer Plugin [![Actions Status](https://github.com/ManiVaultStudio/SpectralViewPlugin/actions/workflows/build.yml/badge.svg)](https://github.com/ManiVaultStudio/SpectralViewPlugin/actions)
Viewer plugin for hyperspectral image data in the [ManiVault](https://github.com/ManiVaultStudio/core) framework.

```bash
git clone git@github.com:ManiVaultStudio/SpectralViewPlugin.git
```
<p align="center">
  <img src="https://github.com/ManiVaultStudio/core/assets/58806453/1957eb15-af49-4e2a-bea5-752a6f1fab5c" alt="t-SNE and HSNE embeddings">
  Spectral viewer (center bottom) together with an image viewer and scatterplot, showing the average spectra of three sets of pixels (clusters in the t-SNE embedding). The image viewer shows a false-color representation of the data - the spectral viewer is used to set the wavelengths that are mapped to the RGB channels.
</p>

## Usage
How to use:

0. Load a data set, e.g. with the [ENVI](https://github.com/ManiVaultStudio/ENVILoader) or [general image](https://github.com/ManiVaultStudio/ImageLoaderPlugin) loader plugins. **Caveat**: It's encouraged to define dimension names that encode the wavelength of the respective image channel, e.g. "304.7", "1020" or "304.7 nm". This wavelength information will be displayed in the viewer. If no dimension names are given when loading the data, ManiVault automatically numbers the dimensions, i.e. "Dim 0", etc.; in this case, the dimension numbers are displayed on the x-Axis instead of wavelengths.
1. Open a spectral viewer and drag & drop the point data into the viewer. At this point you will not see anything yet. Open data in another viewer, e.g. an image viewer, and make a selection. Now you'll see the average spectral values for the selected points.
2. Create cluster for your data set, e.g. using an analysis like [mean shift clustering](https://github.com/ManiVaultStudio/MeanShiftClustering) or through manual annotation in the [scatterplot](https://github.com/ManiVaultStudio/Scatterplot). Drag & drop the cluster data into the main view of the spectral viewer.
3. Perform a mapping like the "Spectral Angle Mapping" algorithm for a selected cluster. This will cerate a new image data set. 

## References
Based on Popa et al. "Visual Analysis of RIS Data for Endmember Selection" (2022):

```bibtex
@inproceedings {10.2312:gch.20221233,
    booktitle = {Eurographics Workshop on Graphics and Cultural Heritage},
    editor = {Ponchio, Federico and Pintus, Ruggero},
    title = {{Visual Analysis of RIS Data for Endmember Selection}},
    author = {Popa, Andra and Gabrieli, Francesca and Kroes, Thomas and Krekeler, Anna and Alfeld, Matthias and Lelieveldt, Boudewijn and Eisemann, Elmar and Höllt, Thomas},
    year = {2022},
    publisher = {The Eurographics Association},
    ISSN = {2312-6124},
    ISBN = {978-3-03868-178-6},
    DOI = {10.2312/gch.20221233}
}
```
