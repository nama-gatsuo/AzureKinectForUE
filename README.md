# Azure Kinect for Unreal Engine

![](./Docs/kinect.png)

Exposes Azure Kinect Support for integration into Unreal Engine Applications.
Mainly for depth and color textures creation from Kinect's raw feed.

## Prerequisites

* Platform: Win64
* Azure Kinect SDK `Azure Kinect SDK v1.4.1` is installed
    * Download frome [here](https://github.com/microsoft/Azure-Kinect-Sensor-SDK/blob/develop/docs/usage.md)
    * Also an env variable `AZUREKINECT_SDK` that points to the Azure Kinect SDK root path should be registered. Otherwise this plugin cannot be neither built or open. 
* Unreal Engine 4.27~
    * Only tested with 4.27. May work with lower.

## Features

### In-Editor activation

* Write Depth / Color buffer into `RenderTarget2D`s. 

![](./Docs/in-editor.gif)

### Blueprint activation

![](./Docs/bp.png)

### Niagara Particle

* You can modify base a niagara system `NS_KinectParticle`.

![](./Docs/animation.gif)

# Reference

Existing plugin for Azure Kinect
* [secretlocation/azure-kinect-unreal](https://github.com/https://github.com/secretlocation/azure-kinect-unreal/azure-kinect-unreal)
    * Body tracking only, not support point cloud and texture(s)
    * Referred a lot from this repo
