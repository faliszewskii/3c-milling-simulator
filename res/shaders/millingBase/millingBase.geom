#version 450

layout(points) in; // Expecting one point per instance
layout(triangle_strip, max_vertices = 6) out;

uniform sampler2D uHeightMap;
uniform sampler2D uModelHeightMap;
uniform vec2 uGridSize;    // n by m grid
uniform vec2 uBaseSize;
uniform float uMaxHeight;  // Maximum height for the cubes
uniform float uHeightScale; // Scale of the height
uniform bool useColorMap;

uniform mat4 view;
uniform mat4 projection;

in vec3 vPosition[]; // Passed from vertex shader (local position of cube)
flat in int instanceID[];  // Instance ID passed from vertex shader

out vec3 normal;
out vec3 fragPos;
out vec2 texCoords;
out float color;

void CreateQuadWall(vec3 position1, vec3 position2, vec3 norm) {
    vec2 cubeSize = 1.f / uGridSize * uBaseSize;
    vec2 cubeSizeHalf = cubeSize / 2.f;
    vec3 cubeVertices[4] = vec3[](
        vec3(position2.x, 0, position2.z), // 5: right-back
        vec3(position2),  // 7: left-front
        vec3(position1), // 6: right-front
        vec3(position1.x, 0, position1.z) // 4: left-back
    );

    int indices[6] = int[](
    // Top face (two triangles, clockwise from above)
    0, 3, 2, 0, 2, 1
    );

    // Emit cube vertices as triangle strips
    for (int i = 0; i < 6; i++) {
        gl_Position = projection * view * vec4(cubeVertices[indices[i]], 1);
        fragPos = cubeVertices[indices[i]];
        normal = norm;
        color = 1;
        texCoords = vec2(0,0);
        EmitVertex();
        if (i % 3 == 2) EndPrimitive(); // End each quad
    }
}

void CreateQuad(vec3 position, vec4 height, vec3 norm1, vec3 norm2, vec4 colors) {
    vec2 cubeSize = 1.f / uGridSize * uBaseSize;
    vec2 cubeSizeHalf = cubeSize / 2.f;
    vec3 cubeVertices[4] = vec3[](
    vec3(-cubeSizeHalf.x, height.x, -cubeSizeHalf.y), // 4: Top-left-back
    vec3( cubeSizeHalf.x, height.y, -cubeSizeHalf.y), // 5: Top-right-back
    vec3( cubeSizeHalf.x, height.z,  cubeSizeHalf.y), // 6: Top-right-front
    vec3(-cubeSizeHalf.x, height.w,  cubeSizeHalf.y)  // 7: Top-left-front
    );

    norm1 = normalize(cross(cubeVertices[3]-cubeVertices[0], cubeVertices[2]-cubeVertices[0]));
    norm2 = normalize(cross(cubeVertices[2]-cubeVertices[0], cubeVertices[1]-cubeVertices[0]));

    int indices[6] = int[](
    // Top face (two triangles, clockwise from above)
    0, 3, 2, 0, 2, 1
    );

    float colorsA[6] = float[](
        colors[0], colors[3], colors[2], colors[0], colors[2], colors[1]
    );

    // Emit cube vertices as triangle strips
    for (int i = 0; i < 6; i++) {
        gl_Position = projection * view * vec4(position + cubeVertices[indices[i]], 1);
        fragPos = position + cubeVertices[indices[i]];
        normal = i < 3 ? norm1: norm2;
        color = colorsA[i];
        texCoords = vec2(0,0);
        EmitVertex();
        if (i % 3 == 2) EndPrimitive(); // End each quad
    }
}
vec4 cubic(float v){
    vec4 n = vec4(1.0, 2.0, 3.0, 4.0) - v;
    vec4 s = n * n * n;
    float x = s.x;
    float y = s.y - 4.0 * s.x;
    float z = s.z - 4.0 * s.y + 6.0 * s.x;
    float w = 6.0 - x - y - z;
    return vec4(x, y, z, w) * (1.0/6.0);
}

vec4 textureBicubic(sampler2D samp, vec2 texCoords){

    ivec2 texSize = textureSize(samp, 0);
    vec2 invTexSize = 1.0 / vec2(texSize);

    texCoords = texCoords * texSize - 0.5;


    vec2 fxy = fract(texCoords);
    texCoords -= fxy;

    vec4 xcubic = cubic(fxy.x);
    vec4 ycubic = cubic(fxy.y);

    vec4 c = texCoords.xxyy + vec2 (-0.5, +1.5).xyxy;

    vec4 s = vec4(xcubic.xz + xcubic.yw, ycubic.xz + ycubic.yw);
    vec4 offset = c + vec4 (xcubic.yw, ycubic.yw) / s;

    offset *= invTexSize.xxyy;

    vec4 sample0 = texture(samp, offset.xz);
    vec4 sample1 = texture(samp, offset.yz);
    vec4 sample2 = texture(samp, offset.xw);
    vec4 sample3 = texture(samp, offset.yw);

    float sx = s.x / (s.x + s.y);
    float sy = s.z / (s.z + s.w);

    return mix(mix(sample3, sample2, sx), mix(sample1, sample0, sx), sy);
}


void main() {
    // Compute grid position based on instance ID
    int row = instanceID[0] / int(uGridSize.x+2);  // Y grid coordinate
    int col = instanceID[0] % int(uGridSize.x+2);  // X grid coordinate

    vec2 cubeSize = 1.f  / uGridSize * uBaseSize;
    vec2 cubeSizeHalf = cubeSize / 2.f;

    // Map to grid position in world space
    if(row == 0 || row == uGridSize.y+1 || col == 0 || col == uGridSize.x+1) {
        if((row == 0 && col == 0 ) || (row == 0 && col == uGridSize.x+1) || (row == uGridSize.y+1 && col == 0 ) || (row == uGridSize.y+1 && col == uGridSize.x+1)) {
            return;
        }
        if(row == 0) {
            vec3 gridPosition = vec3(float(col-1)/uGridSize.x * uBaseSize.x - uBaseSize.x/2.f, 0.0, float(row)/uGridSize.y * uBaseSize.y - uBaseSize.y/2.f - cubeSizeHalf.y);
            vec2 uv;
            uv = vec2(float(col-1 - 0.5) / uGridSize.x, float(row) / uGridSize.y);
            uv = vec2(uv.y, uv.x);
            float height1 = textureBicubic(uHeightMap, uv).r * uHeightScale;
            uv = vec2(float(col-1 + 0.5) / uGridSize.x, float(row) / uGridSize.y);
            uv = vec2(uv.y, uv.x);
            float height2 = textureBicubic(uHeightMap, uv).r * uHeightScale;
            CreateQuadWall(
                vec3(gridPosition.x - cubeSizeHalf.x, height1, gridPosition.z),
                vec3(gridPosition.x + cubeSizeHalf.x, height2, gridPosition.z),
                vec3(0, 0, -1)
            );
        }
        if(col == 0) {
            vec3 gridPosition = vec3(float(col)/uGridSize.x * uBaseSize.x - uBaseSize.x/2.f - cubeSizeHalf.x, 0.0, float(row-1)/uGridSize.y * uBaseSize.y - uBaseSize.y/2.f);
            vec2 uv;
            uv = vec2(float(col) / uGridSize.x, float(row-1-0.5) / uGridSize.y);
            uv = vec2(uv.y, uv.x);
            float height1 = textureBicubic(uHeightMap, uv).r * uHeightScale;
            uv = vec2(float(col) / uGridSize.x, float(row-1+0.5) / uGridSize.y);
            uv = vec2(uv.y, uv.x);
            float height4 = textureBicubic(uHeightMap, uv).r * uHeightScale;
            CreateQuadWall(
                vec3(gridPosition.x, height4, gridPosition.z+cubeSizeHalf.y),
                vec3(gridPosition.x, height1, gridPosition.z-cubeSizeHalf.y),
                vec3(-1,0,0)
            );
        }
        if(row == uGridSize.y+1) {
            vec3 gridPosition = vec3(float(col-1)/uGridSize.x * uBaseSize.x - uBaseSize.x/2.f, 0.0, float(row-2)/uGridSize.y * uBaseSize.y - uBaseSize.y/2.f + cubeSizeHalf.y);
            vec2 uv;
            uv = vec2(float(col-1+0.5) / uGridSize.x, float(row-2) / uGridSize.y);
            uv = vec2(uv.y, uv.x);
            float height3 = textureBicubic(uHeightMap, uv).r * uHeightScale;
            uv = vec2(float(col-1-0.5) / uGridSize.x, float(row-2) / uGridSize.y);
            uv = vec2(uv.y, uv.x);
            float height4 = textureBicubic(uHeightMap, uv).r * uHeightScale;
            CreateQuadWall(
                vec3(gridPosition.x + cubeSizeHalf.x, height3, gridPosition.z),
                vec3(gridPosition.x - cubeSizeHalf.x, height4, gridPosition.z),
                vec3(0,0,1)
            );
        }
        if(col == uGridSize.x+1) {
            vec3 gridPosition = vec3(float(col-2)/uGridSize.x * uBaseSize.x - uBaseSize.x/2.f + cubeSizeHalf.x, 0.0, float(row-1)/uGridSize.y * uBaseSize.y - uBaseSize.y/2.f);
            vec2 uv;
            uv = vec2(float(col-2) / uGridSize.x, float(row-1-0.5) / uGridSize.y);
            uv = vec2(uv.y, uv.x);
            float height2 = textureBicubic(uHeightMap, uv).r * uHeightScale;
            uv = vec2(float(col-2) / uGridSize.x, float(row-1+0.5) / uGridSize.y);
            uv = vec2(uv.y, uv.x);
            float height3 = textureBicubic(uHeightMap, uv).r * uHeightScale;
            CreateQuadWall(
                vec3(gridPosition.x, height2, gridPosition.z-cubeSizeHalf.y),
                vec3(gridPosition.x, height3, gridPosition.z+cubeSizeHalf.y),
                vec3(1,0,0)
            );
        }
        return;
    }
    row--;
    col--;
    vec3 gridPosition = vec3(float(col)/uGridSize.x * uBaseSize.x - uBaseSize.x/2.f, 0.0, float(row)/uGridSize.y * uBaseSize.y - uBaseSize.y/2.f);

    vec2 uv;
    vec2 uvModel;
    uv = vec2(float(col-0.5) / uGridSize.x, float(row-0.5) / uGridSize.y);
    uv = vec2(uv.y, uv.x);
    float height1 = textureBicubic(uHeightMap, uv).r * uHeightScale;
    uvModel.x = uv.y;
    uvModel.y = uv.x;
    float target1 = textureBicubic(uModelHeightMap, uvModel).r ;
    uv = vec2(float(col+0.5) / uGridSize.x, float(row-0.5) / uGridSize.y);
    uv = vec2(uv.y, uv.x);
    uvModel.x = uv.y;
    uvModel.y = uv.x;
    float height2 = textureBicubic(uHeightMap, uv).r * uHeightScale;
    float target2 = textureBicubic(uModelHeightMap, uvModel).r ;
    uv = vec2(float(col+0.5) / uGridSize.x, float(row+0.5) / uGridSize.y);
    uv = vec2(uv.y, uv.x);
    uvModel.x = uv.y;
    uvModel.y = uv.x;
    float height3 = textureBicubic(uHeightMap, uv).r * uHeightScale;
    float target3 = textureBicubic(uModelHeightMap, uvModel).r ;
    uv = vec2(float(col-0.5) / uGridSize.x, float(row+0.5) / uGridSize.y);
    uv = vec2(uv.y, uv.x);
    uvModel.x = uv.y;
    uvModel.y = uv.x;
    float height4 = textureBicubic(uHeightMap, uv).r * uHeightScale;
    float target4 = textureBicubic(uModelHeightMap, uvModel).r;

    vec4 colors = vec4(
         (height1-15)/35 - (1-target1),
         (height2-15)/35 - (1-target2),
         (height3-15)/35 - (1-target3),
         (height4-15)/35 - (1-target4)
    );

    // Vertex positions with heights
    vec3 p1 = vec3(float(col-0.5) / uGridSize.x, float(row-0.5) / uGridSize.y, height1);
    vec3 p2 = vec3(float(col+0.5) / uGridSize.x, float(row-0.5) / uGridSize.y, height2);
    vec3 p3 = vec3(float(col+0.5) / uGridSize.x, float(row+0.5) / uGridSize.y, height3);
    vec3 p4 = vec3(float(col-0.5) / uGridSize.x, float(row+0.5) / uGridSize.y, height4);

    // Compute normals for the two triangles
    vec3 normalTri1 = normalize(cross(p4 - p1, p3 - p1)); // Triangle 1: p1, p4, p3
    vec3 normalTri2 = normalize(cross(p3 - p1, p2 - p1)); // Triangle 2: p1, p3, p2


    CreateQuad(gridPosition, vec4(height1, height2, height3, height4), normalTri1, normalTri2, colors);
}
