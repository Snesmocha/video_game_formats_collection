# video_game_formats_collection
screw you khronos group and screw you amd 

# TFSS (Texture File Storage System) Format Specification

## File Header

The TFSS file begins with a fixed-size header:

| Field        | Size      | Description                                                                 |
|--------------|-----------|-----------------------------------------------------------------------------|
| `magic`      | 4 bytes   | File identifier, must be `'TFSS'`.                                          |
| `flags`      | 4 bytes   | Custom flags for texture usage or storage options.                          |
| `width`      | 4 bytes   | Width of the texture in pixels.                                             |
| `height`     | 4 bytes   | Height of the texture in pixels.                                            |
| `depth`      | 4 bytes   | Depth of the texture, or number of layers in an array.                      |
| `format`     | 1 byte    | Texture format (e.g., RGBA8, RGBA4444, etc.).                              |
| `compression`| 1 byte    | Compression method used on texture data (0 = none, other values = compressed). |
| `mip_count`  | 1 byte    | Number of mipmap levels. 0 = no mipmaps, 1 = one mipmap level, etc.        |
| `face_count` | 1 byte    | Number of faces (1 for 2D textures, 6 for cubemaps).                       |
| `offsets`    | variable  | Array of `additional_metadata` structures indicating the offset of each image block. |

> **Note:** `offsets` allows fast seeking to individual mipmaps, array layers, or faces.

---

## Image Data Storage

Image data is stored sequentially after the header:

1. **Mipmap Levels**
   - Each texture layer stores its mipmaps consecutively.
   - The first mipmap corresponds to the full resolution (`width × height × depth`).
   - Each subsequent mipmap is smaller (usually halved in each dimension) and stored immediately after the previous level.
   - The total size of a layer's mipmaps can be calculated from `mip_count`.

2. **Texture Array / Depth Slices**
   - If `depth > 1`, multiple layers are stored consecutively, each with its full set of mipmaps:
     ```
     Layer 0 mipmaps → Layer 1 mipmaps → Layer 2 mipmaps → ...
     ```

3. **Cubemap Faces**
   - If `face_count = 6`, the six faces of the cubemap are stored sequentially.
   - Each face has its own mipmaps and optional array layers.

4. **Offsets**
   - `additional_metadata` entries store the file offset for each layer, mipmap, or face for efficient access.

---

## Example Layout

Header (TFSS)
├─ Width, Height, Depth, Format, Compression, MipCount, FaceCount
├─ Offsets Table
Image Data
├─ Layer 0
│ ├─ Mip 0
│ ├─ Mip 1
│ └─ Mip 2
├─ Layer 1
│ ├─ Mip 0
│ ├─ Mip 1
│ └─ Mip 2
└─ Layer 2
├─ Mip 0
├─ Mip 1
└─ Mip 2

> **Important:** Mipmaps always follow the texture they belong to.  
> For `mip_count = 0`, only the base image is stored.

---

## Notes

- Zero mipmaps (`mip_count = 0`) means only the base image is stored.  
- Single-layer textures (`depth = 1`) contain only one image or cubemap face sequence.  
- Sequential storage ensures linear reading of all mipmaps or layers.  
- Offset table allows random access to specific mipmaps, faces, or layers.