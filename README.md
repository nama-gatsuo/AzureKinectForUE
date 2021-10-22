# Azure Kinect for Unreal Engine

![](./Docs/kinect.png)

Exposes Azure Kinect Support for integration into Unreal Engine Applications.
Mainly for depth and color textures creation from Kinect's raw feed.

## Prerequisites

* Platform: Win64
* Dependencies:
    * `Azure Kinect SDK v1.4.1` is installed
        * Download from [here](https://github.com/microsoft/Azure-Kinect-Sensor-SDK/blob/develop/docs/usage.md)
        * An env variable `AZUREKINECT_SDK` that points to the Azure Kinect SDK root path should be registered. 
    * `Azure Kinect Body Tracking SDK v1.1.0` is installed
        * Download from [here](https://docs.microsoft.com/en-us/azure/Kinect-dk/body-sdk-download)
        * An env variable `AZUREKINECT_BODY_SDK` that points to the Azure Kinect Body Tracking SDK root path should be registered. 
    * this plugin cannot be neither built nor open without the SDKs and paths above. 
        * [AzureKinect.Build.cs](https://github.com/nama-gatsuo/AzureKinectForUE/blob/master/Source/AzureKinect/AzureKinect.Build.cs) describes how it resolve dependent paths.
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


### Skeleton tracking

![](./Docs/skeletonAnim.gif)

* Bone mapping node in Anim Graph

![](./Docs/animgraph.jpg)

## Notice

Depthe data are stored `RenderTarget2D` into standard 8bit RGBA texture.  
R: first 8bit as `uint8` of original `uint16` sample  
G: last 8bit as `uint8` of original `uint16` sample  
B: `0x00` or `0xFF` (if depth sample is invalid)  
A: `0xFF` (Constant value)

Thus we need conversion to acquire orignal depth samples.
```
// In MaterialEditor or Niagara, sample values in Depth texture are normalized to 0-1.
float DepthSample = (G * 256.0 + R) * 256.0; // millimetor
```

```
// In C++
uint8 R = Sample.R, G = Sample.G;
uint16 DepthSample = G << 8 | R;  // millimetor
```

Depth pixel from Azure Kinect SDK is originally a single `uint16` in millimetor. But `RenderTarget2D` can't store `uint16` as texture (`EPixelFormat::PF_R16_UINT` doesn't work for RenderTarget). 


# Reference

Existing plugin for Azure Kinect
* [secretlocation/azure-kinect-unreal](https://github.com/secretlocation/azure-kinect-unreal/)
    * Body tracking only, not support point cloud and texture(s)
    * Referred a lot from this repo
