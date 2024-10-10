#version 450

layout(points) in; // Expecting one point per instance
layout(triangle_strip, max_vertices = 6) out;

uniform sampler2D uHeightMap;
uniform vec2 uGridSize;    // n by m grid
uniform vec2 uBaseSize;
uniform float uMaxHeight;  // Maximum height for the cubes
uniform float uHeightScale; // Scale of the height

uniform mat4 view;
uniform mat4 projection;

in vec3 vPosition[]; // Passed from vertex shader (local position of cube)
flat in int instanceID[];  // Instance ID passed from vertex shader

out vec3 normal;
out vec3 fragPos;
out vec2 texCoords;

void CreateQuad(vec3 position, vec4 height, vec3 norm) {
    vec2 cubeSize = 1.f / uGridSize * uBaseSize;
    vec2 cubeSizeHalf = cubeSize / 2.f;
    vec3 cubeVertices[4] = vec3[](
    vec3(-cubeSizeHalf.x, height.x, -cubeSizeHalf.y), // 4: Top-left-back
    vec3( cubeSizeHalf.x, height.y, -cubeSizeHalf.y), // 5: Top-right-back
    vec3( cubeSizeHalf.x, height.z,  cubeSizeHalf.y), // 6: Top-right-front
    vec3(-cubeSizeHalf.x, height.w,  cubeSizeHalf.y)  // 7: Top-left-front
    );


    int indices[6] = int[](
    // Top face (two triangles, clockwise from above)
    0, 3, 2, 0, 2, 1
    );

    // Emit cube vertices as triangle strips
    for (int i = 0; i < 6; i++) {
        gl_Position = projection * view * vec4(position + cubeVertices[indices[i]], 1);
        fragPos = position + cubeVertices[indices[i]];
        normal = norm;
        texCoords = vec2(0,0);
        EmitVertex();
        if (i % 3 == 2) EndPrimitive(); // End each quad
    }
}

void main() {
    // Compute grid position based on instance ID
    int row = instanceID[0] / int(uGridSize.x);  // Y grid coordinate
    int col = instanceID[0] % int(uGridSize.x);  // X grid coordinate

    vec2 cubeSize = 1.f  / uGridSize * uBaseSize;
    vec2 cubeSizeHalf = cubeSize / 2.f;

    // Map to grid position in world space
    vec3 gridPosition = vec3(float(col)/uGridSize.x * uBaseSize.x - uBaseSize.x/2.f, 0.0, float(row)/uGridSize.x * uBaseSize.y - uBaseSize.y/2.f);


    vec2 uv;
    uv = vec2(float(col-0.5) / uGridSize.x, float(row-0.5) / uGridSize.y);
    float height1 = texture(uHeightMap, uv).r * uHeightScale;
    uv = vec2(float(col+0.5) / uGridSize.x, float(row-0.5) / uGridSize.y);
    float height2 = texture(uHeightMap, uv).r * uHeightScale;
    uv = vec2(float(col+0.5) / uGridSize.x, float(row+0.5) / uGridSize.y);
    float height3 = texture(uHeightMap, uv).r * uHeightScale;
    uv = vec2(float(col-0.5) / uGridSize.x, float(row+0.5) / uGridSize.y);
    float height4 = texture(uHeightMap, uv).r * uHeightScale;

    // Vertex positions with heights
    vec3 p1 = vec3(float(col-0.5) / uGridSize.x, float(row-0.5) / uGridSize.y, height1);
    vec3 p2 = vec3(float(col+0.5) / uGridSize.x, float(row-0.5) / uGridSize.y, height2);
    vec3 p3 = vec3(float(col+0.5) / uGridSize.x, float(row+0.5) / uGridSize.y, height3);
    vec3 p4 = vec3(float(col-0.5) / uGridSize.x, float(row+0.5) / uGridSize.y, height4);

    // Compute normals for the two triangles
    vec3 normalTri1 = normalize(cross(p2 - p1, p3 - p1)); // Triangle 1: p1, p2, p3
    vec3 normalTri2 = normalize(cross(p3 - p1, p4 - p1)); // Triangle 2: p1, p3, p4

    // Average the two normals to get a smooth normal for the quad
    vec3 norm = normalize(normalTri1 + normalTri2);




    CreateQuad(gridPosition, vec4(height1, height2, height3, height4), norm);
}
