## Brioche Asset Import  

- [ ] Scene  
    - [ ] Mesh  
        - [ ] [MMD (PMX)](https://github.com/MMD-Blender/blender_mmd_tools/tree/main)  
        - [ ] [glTF (glTF, glB, VRM)](https://github.com/saturday06/VRM-Addon-for-Blender/blob/main/src/io_scene_vrm/common/human_bone_mapper/mmd_mapping.py)  
    - [ ] Animation  
        - [ ] [MMD (VMD)](https://github.com/MMD-Blender/blender_mmd_tools/tree/main)  
        - [ ] [glTF (glTF, glB, VRMA)](https://github.com/saturday06/VRM-Addon-for-Blender/blob/main/src/io_scene_vrm/common/human_bone_mapper/mmd_mapping.py)     
- [ ] Image  
    - [ ] [Albedo Image](https://www.pbr-book.org/4ed/Radiometry,_Spectra,_and_Color/Color#FromRGBtoSpectra)  
        - [x] [WebP](https://chromium.googlesource.com/webm/libwebp)  
        - [x] [PNG](https://github.com/pnggroup/libpng)  
        - [x] [JPEG](https://github.com/libjpeg-turbo/libjpeg-turbo)  
    - [ ] [Illuminant Image](https://www.pbr-book.org/4ed/Radiometry,_Spectra,_and_Color/Color#x6-RGBIlluminants)  
        - [ ] [OpenEXR](https://github.com/AcademySoftwareFoundation/openexr)  
- [ ] Environment Map (Environment Lighting Image)  
    - [ ] [Equirectangular (Latitude-Longitude) Map](https://www.pbr-book.org/3ed-2018/Light_Sources/Infinite_Area_Lights)  
    - [ ] [Octahedral Map](https://www.pbr-book.org/4ed/Light_Sources/Infinite_Area_Lights#ImageInfiniteLights)  
    - [ ] [~~Cube Map~~](https://dev.epicgames.com/documentation/en-us/unreal-engine/creating-cubemaps?application_version=4.27)  


### MMD PMX Bone Constraint  

[TDA Miku Append](https://mikumikudance.fandom.com/wiki/Miku_Hatsune_Append_(Tda))  

```graphviz  
digraph 
{
    graph [bgcolor=gray20];
    node [style=filled, fillcolor=gray20, fontcolor=white, color=white];
    edge [fontcolor=white, color=white];
    
    rankdir=LR;

    Shadow_WaistCancel_R_Waist[label="腰", shape=box]
    WaistCancel_R[label="腰キャンセル右", shape=box]

    Shadow_WaistCancel_R_Waist -> WaistCancel_R[label="Copy Local Space Rotation \n Order: 0", headport=w, tailport=e, splines=curved, style=solid]

    subgraph
    {
        rank=same;
        WaistCancel_R -> LegIK_R [constraint=false, style=invis];
    }
    
    WaistCancel_R -> Leg_R [label="Model Space Transform Propagation \n Order: 1", headport=n, tailport=e, splines=curved, style=dotted];

    Shadow_WaistCancel_L_Waist[label="腰", shape=box]
    WaistCancel_L[label="腰キャンセル左", shape=box]

    Shadow_WaistCancel_L_Waist -> WaistCancel_L[label="Copy Local Space Rotation \n Order: 0", headport=w, tailport=e, splines=curved, style=solid]

    subgraph
    {
        rank=same;
        WaistCancel_L -> LegIK_L [constraint=false, style=invis];
    }
    
    WaistCancel_L -> Leg_L [label="Model Space Transform Propagation \n Order: 1", headport=n, tailport=e, splines=curved, style=dotted];

    subgraph
    {
        rank=same;
        Shadow_WaistCancel_R_Waist -> Shadow_WaistCancel_L_Waist [constraint=false, style=invis];
    }

    LegIK_R [label="右足IK", shape=box];

    Leg_R [label="右足", shape=box];
    Knee_R [label="右ひざ", shape=box];
    Ankle_R [label="右足首", shape=box];
    
    LegIK_R -> Leg_R [label="右足: Reaching IK Root Joint (Local Space Rotation Modified) \n Order: 1", headport=s, tailport=n, splines=curved, style=solid];
    LegIK_R -> Knee_R [label="右ひざ: Reaching IK Intermediate Joint (Local Space Rotation Modified) \n Order: 1", headport=n, tailport=n, splines=curved, style=solid];
    LegIK_R -> Ankle_R [label="右足IK: Reaching IK Target Position \n 右足首: Reaching IK End Effector (Local Space Transform Preserved) \n Order: 1", headport=n, tailport=s, splines=curved, style=dashed];
    
    subgraph 
    {
        rank=same;
        Leg_R -> Knee_R [constraint=false, style=invis];
        Knee_R -> Ankle_R [constraint=false, style=invis];
    }
    
    ToeTipIK_R [label="右つま先IK", shape=box];
    
    ToeTip_R [label="右つま先", shape=box];
    
    ToeTipIK_R -> Ankle_R [label="右足首: Reaching IK Root Joint (Local Space Rotation Modifier) \n Order: 2", headport=s, tailport=n, splines=curved, style=solid];
    ToeTipIK_R -> ToeTip_R [label="右つま先IK: Reaching IK Target Position \n 右つま先: Reaching IK End Effector (Local Space Transform Preserved) \n Order: 2", headport=s, tailport=s, splines=curved, style=dashed];

    subgraph 
    {
        rank=same;
        Ankle_R -> ToeTip_R [constraint=false, style=invis];
    }

    subgraph
    {
        rank=same; 
        LegIK_R -> ToeTipIK_R[constraint=false, style=invis];
    }

    LegD_R [label="右足D", shape=box];
    
    Leg_R -> LegD_R [label="Copy Local Space Rotation \n Order: 3", headport=w, tailport=s, splines=curved, style=solid]
    
    KneeD_R [label="右ひざD", shape=box];

    Knee_R -> KneeD_R [label="Copy Local Space Rotation \n Order: 3", headport=w, tailport=n, splines=curved, style=solid]

    AnkleD_R [label="右足首D", shape=box];

    Ankle_R -> AnkleD_R [label="Copy Local Space Rotation \n Order: 3", headport=w, tailport=s, splines=curved, style=solid]
    
    LegIK_L [label="左足IK", shape=box];

    Leg_L [label="左足", shape=box];
    Knee_L [label="左ひざ", shape=box];
    Ankle_L [label="左足首", shape=box];
    
    LegIK_L -> Leg_L [label="左足: Reaching IK Root Joint (Local Space Rotation Modified) \n Order: 1", headport=s, tailport=n, splines=curved, style=solid];
    LegIK_L -> Knee_L [label="左ひざ: Reaching IK Intermediate Joint (Local Space Rotation Modified) \n Order: 1", headport=n, tailport=n, splines=curved, style=solid];
    LegIK_L -> Ankle_L [label="左足IK: Reaching IK Target Position \n 左足首: Reaching IK End Effector (Local Space Transform Preserved) \n Order: 1", headport=n, tailport=s, splines=curved, style=dashed];
    
    subgraph 
    {
        rank=same;
        Leg_L -> Knee_L [constraint=false, style=invis];
        Knee_L -> Ankle_L [constraint=false, style=invis];
    }
    
    ToeTipIK_L [label="左つま先IK", shape=box];
    
    ToeTip_L [label="左つま先", shape=box];
    
    ToeTipIK_L -> Ankle_L [label="左足首: Reaching IK Root Joint (Local Space Rotation Modifier) \n Order: 2", headport=s, tailport=n, splines=curved, style=solid];
    ToeTipIK_L -> ToeTip_L [label="左つま先IK: Reaching IK Target Position \n 左つま先: Reaching IK End Effector (Local Space Transform Preserved) \n Order: 2", headport=s, tailport=s, splines=curved, style=dashed];

    subgraph 
    {
        rank=same;
        Ankle_L -> ToeTip_L [constraint=false, style=invis];
    }
    
    subgraph
    {
        rank=same; 
        LegIK_L -> ToeTipIK_L[constraint=false, style=invis];
    }
    
    LegD_L [label="左足D", shape=box];
    
    Leg_L -> LegD_L [label="Copy Local Space Rotation \n Order: 3", headport=w, tailport=s, splines=curved, style=solid]
    
    KneeD_L [label="左ひざD", shape=box];

    Knee_L -> KneeD_L [label="Copy Local Space Rotation \n Order: 3", headport=w, tailport=n, splines=curved, style=solid]

    AnkleD_L [label="左足首D", shape=box];

    Ankle_L -> AnkleD_L [label="Copy Local Space Rotation \n Order: 3", headport=w, tailport=s, splines=curved, style=solid]
}
```  
