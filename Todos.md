# TODOs

### General Graphics Todos

- [x] Enable Alpha Blending for sprites
- [x] Update orthogonal for camera and test it
- [x] ReTest TGA implementation
- [x] Test out texture height and width with vertex rendering
- [x] Test out dynamic vertex updates
- [x] Make a base Bitmap interface
- [x] Implement Multiple Texture support
- [x] Reimplement slots handling behaviour
- [x] Clean out implementation

### Todos for Sprite

- [x] Sprite that acts with world matrix (transforms in 3D space)
- [x] Sprite to render directly on screen (ignore lighting, ignore world space)
- [x] Sprite to render on back of screen (may respect lighting toggle)

- [x] Sprite Animation System
  - Load frame-based textures
  - Play, pause, and loop control
  - Handle non-looping animation
  - Integrate timing and frame duration

### Graphics Features Todos

- [x] Add more lighting types
- [x] Add ImGui UI overlay (basic setup)
- [ ] Add Font Engine *(skipped for now, too much time required vs. benefit)*
- [x] Multi-Texturing (blend 2 albedos)
- [x] Light Maps
- [x] Alpha Mapping (transparency & blending logic)
- [x] Normal Mapping (per-pixel lighting with tangent space)
- [x] Specular Mapping (per-pixel specular intensity)
- [ ] Projective Texturing *(deferred)*
- [ ] Projected Light Maps *(deferred)*

- [ ] **1. Create shadow depth texture** (`ID3D11Texture2D`) per light  
      - Format: `DXGI_FORMAT_R32_TYPELESS`  
      - BindFlags: `D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE`  
      - MiscFlags: `0` (or use `RESOURCE_MISC_TEXTURECUBE` for point lights later)

- [ ] **2. Create DepthStencilView (DSV)** from the shadow texture  
      - Format: `DXGI_FORMAT_D32_FLOAT`  
      - Used during the shadow pass

- [ ] **3. Create ShaderResourceView (SRV)** from the same texture  
      - Format: `DXGI_FORMAT_R32_FLOAT`  
      - Used in lighting pass to read shadow depth

- [ ] **4. Compute Light View Matrix**  
      - `XMMatrixLookAtLH(lightPos, target, up)`

- [ ] **5. Compute Light Projection Matrix**  
      - Directional: `XMMatrixOrthographicLH()`  
      - Spot light: `XMMatrixPerspectiveFovLH()`  
      - Point light: cube mapping (deferred for now)

- [ ] **6. Combine into LightViewProj matrix**  
      - Multiply View * Projection  
      - Store in constant buffer

- [ ] **7. Create a rasterizer state with depth bias**  
      - `DepthBias`, `SlopeScaledDepthBias`, `DepthBiasClamp`  
      - Optional: `CULL_FRONT` to fix Peter Panning

- [ ] **8. Shadow Map Render Pass**  
      - Bind only DSV (no RTV)  
      - Clear depth buffer  
      - Use minimal vertex shader (position only)  
      - Use null pixel shader  
      - Set viewport to shadow texture resolution  
      - Set rasterizer state with depth bias

- [ ] **9. Render scene from each light's perspective**  
      - For each shadow-casting light:  
        - Bind its DSV  
        - Set LightViewProj  
        - Render all shadow-casting geometry

- [ ] **10. Bind shadow maps in main pass**  
      - Set SRVs for shadow maps in pixel shader  
      - Pass LightViewProj matrix again  
      - Transform world pos → light clip space → UV  
      - Sample shadow map  
      - Compare with fragment depth (+bias)

- [ ] **11. Modulate lighting by shadow visibility**  
      - If in shadow → reduce diffuse/specular  
      - Shadow factor = 0.0 (shadowed), 1.0 (lit)

- [ ] **12. Multiple Light Shadow Mapping**  
      - Maintain arrays for:
        - Shadow textures (DSVs, SRVs)  
        - Light view-proj matrices  
      - In pixel shader:  
        - Loop or index into light array  
        - Blend shadow contributions from all lights  
        - (optional: weight shadows per light type/intensity)

- [ ] **13. Directional Shadow Mapping (Cascaded)**  
      - Split camera frustum into cascades (3–4)  
      - Render a shadow map for each cascade  
      - Store cascade view-proj + split distance  
      - In pixel shader:  
        - Pick correct cascade based on view depth  
        - Sample corresponding shadow map

- [ ] **14. Soft Shadows (PCF - Percentage Closer Filtering)**  
      - In pixel shader:  
        - Sample shadow map in a 3x3 or 5x5 grid around UV  
        - Compare each sample’s depth  
        - Average the result  
      - Simulates soft edge penumbra

- [ ] **15. Visual Debugging (Optional)**  
      - Draw shadow map as a grayscale texture on screen  
      - Visualize depth values, cascade splits, etc.

- [ ] **16. Optimize**  
      - Reduce shadow resolution for distant cascades  
      - Use stencil/culling to limit shadow render  
      - Batch render shadow casters per light  
