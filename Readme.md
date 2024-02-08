
# Spectral Viewer Plugin [![Actions Status](https://github.com/ManiVaultStudio/SpectralViewPlugin/actions/workflows/build.yml/badge.svg)](https://github.com/ManiVaultStudio/SpectralViewPlugin/actions)
Viewer plugin for hyperspectral data in the ManiVault framework.

```bash
git clone git@github.com:ManiVaultStudio/SpectralViewPlugin.git
```
![Spectral Viewer together with an image viewer and scatterplot](https://github.com/ManiVaultStudio/core/assets/58806453/1957eb15-af49-4e2a-bea5-752a6f1fab5c "Spectral Viewer")

## Usage
How to use:

0. Load a data set. **Caveat**: the data must have dimension names that correspond to the wavelength of the respective dimension. E.g. "304.7" and "1020" are valid names but "Dim 01" or "304.7 nm" are not.
1. Open a spectral veiwer and drag & drop the point data into the viewer. At this point you will not see anything yet. Open data in another viewer, e.g. an image viewer, and make a selection. Now you'll see the average spectral values for the selected points.
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
