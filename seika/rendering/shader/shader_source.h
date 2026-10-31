#pragma once

// GLSL 330 core isn't valid under GLES -- Android/Emscripten need "#version 300 es" plus mandatory
// fragment-shader precision qualifiers (vertex shaders default float/int to highp already, so
// precision is only added to the fragment macro).
#if defined(PLATFORM_ANDROID) || defined(PLATFORM_EMSCRIPTEN)
#define SKA_GLSL_VERSION_VERTEX   "#version 300 es\n"
#define SKA_GLSL_VERSION_FRAGMENT "#version 300 es\n" "precision mediump float;\n" "precision mediump sampler2D;\n"
// GLES 3.0 only guarantees 256 vertex uniform vectors and a mat4 takes 4, so 100 fails to link on some GPUs
#define SKA_SPRITE_MAX_MODELS 60
#else
#define SKA_GLSL_VERSION_VERTEX   "#version 330 core\n"
#define SKA_GLSL_VERSION_FRAGMENT "#version 330 core\n"
#define SKA_SPRITE_MAX_MODELS 100
#endif
#define SKA_GLSL_INT_IMPL(VALUE) #VALUE
#define SKA_GLSL_INT(VALUE) SKA_GLSL_INT_IMPL(VALUE)

#if SKA_RENDERING

static const char* ska_opengl_shader_source_vertex_sprite =
    SKA_GLSL_VERSION_VERTEX
    "\n"
    "layout (location = 0) in float ID;\n"
    "layout (location = 1) in vec2 CRE_VERTEX;\n"
    "layout (location = 2) in vec2 CRE_TEXTURE_COORDS;\n"
    "layout (location = 3) in vec4 CRE_TEXTURE_MODULATE;\n"
    "layout (location = 4) in float CRE_APPLY_NEAREST_NEIGHBOR;\n"
    "\n"
    "out vec2 UV;\n"
    "out vec4 TEXTURE_MODULATE;\n"
    "out float USING_NEAREST_NEIGHBOR;\n"
    "\n"
    // Explicit highp: GLSL ES vertex shaders default float to highp, but the fragment shaders
    // here set "precision mediump float" globally -- without an explicit qualifier, TIME ends up
    // highp in one stage and mediump in the other, which strict GLES/WebGL linkers (unlike
    // desktop GL) reject with "not linkable between attached shaders". highp is also just the
    // correct precision for a monotonically-increasing time value regardless -- mediump risks
    // visible precision loss/jitter the longer the app has been running.
    "uniform highp float TIME;\n"
    "uniform sampler2D TEXTURE;\n"
    "uniform mat4 CRE_MODELS[" SKA_GLSL_INT(SKA_SPRITE_MAX_MODELS) "];\n"
    "uniform mat4 CRE_PROJECTION;\n"
    "\n"
    "//@@UNIFORMS\n"
    "\n"
    "//@@FUNCTIONS\n"
    "\n"
    "void main() {\n"
    "    int CRE_SPRITE_INT_ID = int(ID);\n"
    "    UV = CRE_TEXTURE_COORDS;\n"
    "    TEXTURE_MODULATE = CRE_TEXTURE_MODULATE;\n"
    "    USING_NEAREST_NEIGHBOR = CRE_APPLY_NEAREST_NEIGHBOR;\n"
    "    vec2 VERTEX = CRE_VERTEX;\n"
    "    //@@vertex()\n"
    "    gl_Position = CRE_PROJECTION * CRE_MODELS[CRE_SPRITE_INT_ID] * vec4(VERTEX, 0.0f, 1.0f);\n"
    "}\n";

static const char* ska_opengl_shader_source_fragment_sprite =
    SKA_GLSL_VERSION_FRAGMENT
    "\n"
    "in vec2 UV;\n"
    "in vec4 TEXTURE_MODULATE;\n"
    "in float USING_NEAREST_NEIGHBOR;\n"
    "\n"
    "out vec4 COLOR;\n"
    "\n"
    // Explicit highp: GLSL ES vertex shaders default float to highp, but the fragment shaders
    // here set "precision mediump float" globally -- without an explicit qualifier, TIME ends up
    // highp in one stage and mediump in the other, which strict GLES/WebGL linkers (unlike
    // desktop GL) reject with "not linkable between attached shaders". highp is also just the
    // correct precision for a monotonically-increasing time value regardless -- mediump risks
    // visible precision loss/jitter the longer the app has been running.
    "uniform highp float TIME;\n"
    "uniform sampler2D TEXTURE;\n"
    "\n"
    "//@@UNIFORMS\n"
    "\n"
    "vec2 CRE_APPLY_NEAREST_NEIGHBOR(vec2 uv, vec2 texture_size) {\n"
    "    vec2 pixel = uv * texture_size;\n"
    "    vec2 seam = floor(pixel + 0.5);\n"
    "    vec2 dudv = fwidth(pixel);\n"
    "    pixel = seam + clamp( (pixel - seam) / dudv, -0.5, 0.5);\n"
    "    return pixel / texture_size;\n"
    "}\n"
    "\n"
    "//@@FUNCTIONS\n"
    "\n"
    "void main() {\n"
    "    vec2 TEXTURE_SIZE = vec2(textureSize(TEXTURE, 0));\n"
    "    vec2 CRE_SPRITE_UV = mix(UV, CRE_APPLY_NEAREST_NEIGHBOR(UV, TEXTURE_SIZE), USING_NEAREST_NEIGHBOR);\n"
    "    COLOR = TEXTURE_MODULATE * texture(TEXTURE, CRE_SPRITE_UV);\n"
    "    //@@fragment()\n"
    "}\n";

static const char* ska_opengl_shader_source_vertex_font =
    SKA_GLSL_VERSION_VERTEX
    "layout (location = 0) in vec4 vertex; // (pos, tex)\n"
    "\n"
    "out vec2 texCoords;\n"
    "\n"
    "uniform mat4 projection;\n"
    "\n"
    "void main() {\n"
    "    gl_Position = projection * vec4(vertex.xy, 0.0f, 1.0f);\n"
    "    texCoords = vertex.zw;\n"
    "}\n";

static const char* ska_opengl_shader_source_fragment_font =
    SKA_GLSL_VERSION_FRAGMENT
    "in vec2 texCoords;\n"
    "out vec4 color;\n"
    "\n"
    "uniform sampler2D textValue;\n"
    "uniform vec4 textColor;\n"
    "\n"
    "void main() {\n"
    "    vec4 sampled = vec4(1.0f, 1.0f, 1.0f, texture(textValue, texCoords).r);\n"
    "    color = textColor * sampled;\n"
    "}\n";

static const char* ska_opengl_shader_source_vertex_screen =
    SKA_GLSL_VERSION_VERTEX
    "layout (location = 0) in vec2 CRE_VERTEX;\n"
    "layout (location = 1) in vec2 CRE_TEXTURE_COORDS;\n"
    "\n"
    "out vec2 UV;\n"
    "\n"
    // Explicit highp: GLSL ES vertex shaders default float to highp, but the fragment shaders
    // here set "precision mediump float" globally -- without an explicit qualifier, TIME ends up
    // highp in one stage and mediump in the other, which strict GLES/WebGL linkers (unlike
    // desktop GL) reject with "not linkable between attached shaders". highp is also just the
    // correct precision for a monotonically-increasing time value regardless -- mediump risks
    // visible precision loss/jitter the longer the app has been running.
    "uniform highp float TIME;\n"
    "uniform sampler2D TEXTURE;\n"
    "\n"
    "//@@UNIFORMS\n"
    "\n"
    "//@@FUNCTIONS\n"
    "\n"
    "void main() {\n"
    "    UV = CRE_TEXTURE_COORDS;\n"
    "    vec2 VERTEX = CRE_VERTEX;\n"
    "    //@@vertex()\n"
    "    gl_Position = vec4(VERTEX, 0.0f, 1.0f);\n"
    "}\n";

static const char* ska_opengl_shader_source_fragment_screen =
    SKA_GLSL_VERSION_FRAGMENT
    "out vec4 COLOR;\n"
    "\n"
    "in vec2 UV;\n"
    "\n"
    // Explicit highp: GLSL ES vertex shaders default float to highp, but the fragment shaders
    // here set "precision mediump float" globally -- without an explicit qualifier, TIME ends up
    // highp in one stage and mediump in the other, which strict GLES/WebGL linkers (unlike
    // desktop GL) reject with "not linkable between attached shaders". highp is also just the
    // correct precision for a monotonically-increasing time value regardless -- mediump risks
    // visible precision loss/jitter the longer the app has been running.
    "uniform highp float TIME;\n"
    "uniform sampler2D TEXTURE;\n"
    "\n"
    "//@@UNIFORMS\n"
    "\n"
    "//@@FUNCTIONS\n"
    "\n"
    "void main() {\n"
    "    COLOR = texture(TEXTURE, UV);\n"
    "    //@@fragment()\n"
    "}\n";

#endif // #if SKA_RENDERING

static const char* ska_opengl_shader_source_vertex_mesh =
    SKA_GLSL_VERSION_VERTEX
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aNormal;\n"
    "layout (location = 2) in vec2 aTexCoords;\n"
    "\n"
    "out vec3 FragPos;\n"
    "out vec3 Normal;\n"
    "out vec2 TexCoords;\n"
    "\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "\n"
    "void main() {\n"
    "    FragPos = vec3(model * vec4(aPos, 1.0));\n"
    "    Normal = mat3(transpose(inverse(model))) * aNormal;\n"
    "    TexCoords = aTexCoords;\n"
    "    gl_Position = projection * view * vec4(FragPos, 1.0);\n"
    "}\n";

static const char* ska_opengl_shader_source_fragment_mesh =
    SKA_GLSL_VERSION_FRAGMENT
    "out vec4 FragColor;\n"
    "\n"
    "struct Material {\n"
    "    sampler2D diffuse;\n"
    "    sampler2D specular;\n"
    "    float shininess;\n"
    "};\n"
    "\n"
    "struct DirLight {\n"
    "    vec3 direction;\n"
    "\n"
    "    vec3 ambient;\n"
    "    vec3 diffuse;\n"
    "    vec3 specular;\n"
    "};\n"
    "\n"
    "struct PointLight {\n"
    "    vec3 position;\n"
    "\n"
    "    float constant;\n"
    "    float linear;\n"
    "    float quadratic;\n"
    "\n"
    "    vec3 ambient;\n"
    "    vec3 diffuse;\n"
    "    vec3 specular;\n"
    "};\n"
    "\n"
    "struct SpotLight {\n"
    "    vec3 position;\n"
    "    vec3 direction;\n"
    "    float cutOff;\n"
    "    float outerCutOff;\n"
    "\n"
    "    float constant;\n"
    "    float linear;\n"
    "    float quadratic;\n"
    "\n"
    "    vec3 ambient;\n"
    "    vec3 diffuse;\n"
    "    vec3 specular;\n"
    "};\n"
    "\n"
    "#define MAX_DIRECTIONAL_LIGHTS 20\n"
    "#define MAX_POINT_LIGHTS 20\n"
    "#define MAX_SPOT_LIGHTS 20\n"
    "\n"
    "in vec3 FragPos;\n"
    "in vec3 Normal;\n"
    "in vec2 TexCoords;\n"
    "\n"
    "uniform vec3 viewPos;\n"
    "uniform DirLight directionalLights[MAX_DIRECTIONAL_LIGHTS];\n"
    "uniform int numberOfDirectionalLights;\n"
    "uniform PointLight pointLights[MAX_POINT_LIGHTS];\n"
    "uniform int numberOfPointLights;\n"
    "uniform SpotLight spotLights[MAX_SPOT_LIGHTS];\n"
    "uniform int numberOfSpotLights;\n"
    "uniform Material material;\n"
    // Multiplies the lit result -- defaults to opaque white (a no-op) for existing callers that
    // never set it (ska_renderer_queue_mesh_draw/_ex). Lets ska_renderer_queue_mesh_draw_tinted
    // tint a plain diffuse texture (e.g. a solid-color one) without needing a dedicated shader.
    "uniform vec4 tintColor;\n"
    "\n"
    "// function prototypes\n"
    "vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);\n"
    "vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);\n"
    "vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);\n"
    "\n"
    "void main() {\n"
    "    // properties\n"
    "    vec3 norm = normalize(Normal);\n"
    "    vec3 viewDir = normalize(viewPos - FragPos);\n"
    "\n"
    "    // == =====================================================\n"
    "    // Our lighting is set up in 3 phases: directional, point lights and an optional flashlight\n"
    "    // For each phase, a calculate function is defined that calculates the corresponding color\n"
    "    // per lamp. In the main() function we take all the calculated colors and sum them up for\n"
    "    // this fragment's final color.\n"
    "    // == =====================================================\n"
    "    vec3 result = vec3(0.0f);\n"
    "    // phase 1: directional lighting\n"
    "    for(int i = 0; i < numberOfDirectionalLights; i++) {\n"
    "        result += CalcDirLight(directionalLights[i], norm, viewDir);\n"
    "    }\n"
    "    // phase 2: point lights\n"
    "    for(int i = 0; i < numberOfPointLights; i++) {\n"
    "        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);\n"
    "    }\n"
    "    // phase 3: spot light\n"
    "    for(int i = 0; i < numberOfSpotLights; i++) {\n"
    "        result += CalcSpotLight(spotLights[i], norm, FragPos, viewDir);\n"
    "    }\n"
    "\n"
    "    FragColor = vec4(result * tintColor.rgb, tintColor.a);\n"
    "}\n"
    "\n"
    "// calculates the color when using a directional light.\n"
    "vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir) {\n"
    "    vec3 lightDir = normalize(-light.direction);\n"
    "    // diffuse shading\n"
    "    float diff = max(dot(normal, lightDir), 0.0);\n"
    "    // specular shading\n"
    "    vec3 reflectDir = reflect(-lightDir, normal);\n"
    "    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);\n"
    "    // combine results\n"
    "    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));\n"
    "    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));\n"
    "    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));\n"
    "    return (ambient + diffuse + specular);\n"
    "}\n"
    "\n"
    "// calculates the color when using a point light.\n"
    "vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {\n"
    "    vec3 lightDir = normalize(light.position - fragPos);\n"
    "    // diffuse shading\n"
    "    float diff = max(dot(normal, lightDir), 0.0);\n"
    "    // specular shading\n"
    "    vec3 reflectDir = reflect(-lightDir, normal);\n"
    "    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);\n"
    "    // attenuation\n"
    "    float distance = length(light.position - fragPos);\n"
    "    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));\n"
    "    // combine results\n"
    "    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));\n"
    "    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));\n"
    "    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));\n"
    "    ambient *= attenuation;\n"
    "    diffuse *= attenuation;\n"
    "    specular *= attenuation;\n"
    "    return (ambient + diffuse + specular);\n"
    "}\n"
    "\n"
    "// calculates the color when using a spot light.\n"
    "vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {\n"
    "    vec3 lightDir = normalize(light.position - fragPos);\n"
    "    // diffuse shading\n"
    "    float diff = max(dot(normal, lightDir), 0.0);\n"
    "    // specular shading\n"
    "    vec3 reflectDir = reflect(-lightDir, normal);\n"
    "    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);\n"
    "    // attenuation\n"
    "    float distance = length(light.position - fragPos);\n"
    "    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));\n"
    "    // spotlight intensity\n"
    "    float theta = dot(lightDir, normalize(-light.direction));\n"
    "    float epsilon = light.cutOff - light.outerCutOff;\n"
    "    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);\n"
    "    // combine results\n"
    "    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));\n"
    "    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));\n"
    "    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));\n"
    "    ambient *= attenuation * intensity;\n"
    "    diffuse *= attenuation * intensity;\n"
    "    specular *= attenuation * intensity;\n"
    "    return (ambient + diffuse + specular);\n"
    "}\n";

static const char* opengl_shader_source_point_light_vertex =
    SKA_GLSL_VERSION_VERTEX
    "layout (location = 0) in vec3 aPos;\n"
    "\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "\n"
    "void main() {\n"
    "    gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
    "}\n";

static const char* opengl_shader_source_point_light_fragment =
    SKA_GLSL_VERSION_FRAGMENT
    "out vec4 FragColor;\n"
    "\n"
    "void main() {\n"
    "    FragColor = vec4(1.0); // set alle 4 vector values to 1.0\n"
    "}\n";

// Debug line batch: positions are already world-space (callers build line endpoints on the
// CPU), so no model matrix - just view/projection, matching how e.g. chunk meshes already bake
// their own world transform ahead of the draw. Per-vertex color (not a single uniform) so one
// batched draw call can mix differently-colored line segments.
static const char* opengl_shader_source_debug_line_vertex =
    SKA_GLSL_VERSION_VERTEX
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec4 aColor;\n"
    "\n"
    "out vec4 LineColor;\n"
    "\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "\n"
    "void main() {\n"
    "    LineColor = aColor;\n"
    "    gl_Position = projection * view * vec4(aPos, 1.0);\n"
    "}\n";

static const char* opengl_shader_source_debug_line_fragment =
    SKA_GLSL_VERSION_FRAGMENT
    "out vec4 FragColor;\n"
    "\n"
    "in vec4 LineColor;\n"
    "\n"
    "void main() {\n"
    "    FragColor = LineColor;\n"
    "}\n";
