# Overview

**SlicerPathReconstruction** is an extension of [3D Slicer](https://www.slicer.org/) for creating 3D surface models of catheters from a spatially tracked wire (*e.g.* transform updated through SlicerOpenIGTLink). The user specifies which transform corresponds to the wire then records points along the length of the catheters. The main output is a pair of model nodes for each catheter - one model node for storing raw points, and one for storing a surface representation of the catheter.

## Dependencies

- [SlicerMarkupsToModel](https://github.com/SlicerIGT/SlicerMarkupsToModel)

- [SlicerIGT](https://github.com/SlicerIGT)

- [SlicerOpenIGTLink](https://github.com/openigtlink/SlicerOpenIGTLink) (Recommended)

# PathReconstruction Module

This module contains the main functionality for this extension.

![Overview](https://raw.githubusercontent.com/SlicerIGT/SlicerPathReconstruction/master/Screenshots/PathReconstructionGUI.png?raw=true)
> The GUI for PathReconstruction

The **parameter node** is used to store all options settings for the module. The parameter node can also be saved along with a Slicer scene. When the scene is later re-opened, all options and settings should be preserved.

The **sampling transform**'s position will be sampled for the path reconstruction.

The **start recording** button can be used to start and stop recording points from the sampling transform. The software assumes there is exactly one recording per catheter (one button press to start recording, another to stop recording). The output models will be created and updated automatically. You can delete the last pair of output models by pressing the **delete last** button.

The **paths** tab shows all of the output model nodes that have been created by this module. You can also change the fitting parameters that are used to generate the paths (see the [MarkupsToModel](https://github.com/SlicerIGT/SlicerMarkupsToModel) module for more details).

# PathVerification Module

This module is for checking how much catheter paths correspond with one another (*e.g.* tracked paths generated using the *PathReconstruction* module versus paths determined from CT). This module is mainly intended as a development aid, but some researchers may find its functionality useful. Please keep in mind that, due to its experimental nature, its features are not fully supported.

![GUI](https://raw.githubusercontent.com/SlicerIGT/SlicerMarkupsToModel/master/Screenshots/PathVerificationGUI.png?raw=true)
> The GUI for PathVerification

You can set a segmentation (*e.g.* of catheters in a CT scan) in the **segmented paths** selector, and convert each segment into a catheter path by pressing the **export segments to path** button. They will be stored in the **paths to preprocess** (path reconstruction node).

You can 'trim' down the ends of the catheters to the same length by pressing the **trim paths** button. You can shorten them further by setting a **trim amount** and pressing the button. You may want to do this before doing a surface registration of one set of paths to another.

The **refit with moving least squares** button refits the paths using the Moving Least Squares curve fitting algorithm, and gives the tube model very small radius. The small tube radius is to helpful in analyses of accuracy.

You can register sets of paths by specifying **compare** and **reference** path reconstruction nodes and pressing the **register** button. First a fiducial registration is performed based on the endpoints of the catheters. Second a model-to-model registration is done using the iterative closest point algorithm. The output transform is stored in the **CompareToReference** transform. Usually this will succeed as long as the paths are not too different from one another. If the paths are very different from one another, or they are very symmetric, then the registration may not be accurate.

You can generate some metrics regarding the correspondences between the paths by pressing the **compute** button. It will store raw model-to-model distances in the **distances** table node, and store a summary of results in the **summary** table node.

# Disclaimer

*SlicerPathReconstruction*, like 3D Slicer, is for research. It is not intended for clinical use. The user assumes full responsibility to comply with the appropriate regulations.

