# Shadow Mapping Debugging TODO List

## 1. Ensure Shadow Pass Is Rendering Geometry
- [x] Confirm that your `RenderDepthOnly()` function is called per mesh
- [x] Check that it uses the directional light’s **View mul Projection** matrix
- [x] Use **RSSetViewports()** with correct width/height
- [x] Confirm that **OMSetRenderTargets(0, nullptr, dsv)** is called before drawing
- [x] Disable backface culling temporarily (`CullMode = D3D11_CULL_NONE`) to debug

## 2. Check Light Matrices
- [ ] In CPU code, validate:
  - View matrix looks like a real camera (no NaNs, no identity unless expected)
  - Projection matrix has correct near/far plane range (no 0 or negative values)
- [ ] In shader, ensure you're using:
  ```hlsl
  float4 posLS = mul(float4(posWS, 1.0f), gLightViewProj);
  ```

## 3. Use RenderDoc to Confirm Depth Writes
- [ ] Open your directional light shadow draw call in RenderDoc
- [ ] In the **“Resources”** tab, find the bound Depth Target
- [ ] Click “View Texture” to enable **R32_FLOAT** view
- [ ] Check for:
  - Varying values (shouldn't all be 1.0)
  - Object shapes in grayscale from light’s POV
- [ ] Hover over texture then check pixel values (e.g., 0.2, 0.8, etc.)

## 4. Unbind SRVs Before Writing to DSV
- [ ] At the start of your shadow pass:
  ```cpp
  ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
  context->PSSetShaderResources(15, 1, nullSRV);
  context->PSSetShaderResources(17, 1, nullSRV); // if spot light later
  ```

## 5. Verify Texture and SRV Creation
- [ ] Confirm:
  - `DXGI_FORMAT_R32_TYPELESS` for Texture2D
  - `DXGI_FORMAT_R32_FLOAT` for SRV
  - `DXGI_FORMAT_D32_FLOAT` for DSV
- [ ] Ensure `Texture2DArray` size is correct (width/height/slices)
- [ ] In RenderDoc, inspect the SRV and DSV to confirm they point to **same resource**

## 6. Visual Debug Geometry
- [ ] Place a cube/sphere at (0, 0, 0)
- [ ] Move directional light to clearly cast onto that object
- [ ] Make the scene simple then single shadow-casting object

## 7. Check Binding Order
- [ ] Shadow map pass:
  - Set DSV
  - Set viewport
  - Draw geometry
- [ ] Lighting pass:
  - Bind shadow SRV at `t15`
  - Sample via `Texture2DArray::SampleCmpLevelZero(...)`

## 8. Debug Sampler in HLSL
- [ ] Confirm your HLSL sampler:
  ```hlsl
  SamplerComparisonState gShadowSampler : register(s3);
  ```
  with
  ```cpp
  D3D11_COMPARISON_LESS
  ```
- [ ] Make sure PCF logic uses correct light-space coords:
  ```hlsl
  float shadow = gDirectionalShadowMaps.SampleCmpLevelZero(
      gShadowSampler, lightTexCoord.xy, lightTexCoord.z);
  ```

## 9. Clear DSV Before Writing
- [ ] In each slice of the array:
  ```cpp
  context->ClearDepthStencilView(dsv[i], D3D11_CLEAR_DEPTH, 1.0f, 0);
  ```

## 10. View the Final Shadow Texture in RenderDoc
- [ ] Go to SRV bound at `t15`
- [ ] Select slice 0 (or appropriate one)
- [ ] Confirm it is **not solid red or white**
- [ ] Matches what DSV wrote in previous pass

## Optional Debug Tricks
- [ ] Visualize shadow UVs in pixel shader:
  ```hlsl
  return float4(lightTexCoord.xy, lightTexCoord.z, 1.0f);
  ```
- [ ] Output raw depth:
  ```hlsl
  return gDirectionalShadowMaps.SampleLevel(gShadowSampler, lightTexCoord.xy, 0.0f);
  ```

## Cleanup
- [ ] After lighting pass, unbind the shadow map again if needed
