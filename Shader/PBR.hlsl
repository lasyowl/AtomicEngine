static const float3 DEFAULT_ALBEDO = float3(0.6f, 0.3f, 0.4f);
static const float DEFAULT_METALLIC = 0.0f;
static const float DEFAULT_ROUGH = 0.85f;
static const float DEFAULT_AO = 1.0f;
static const float3 DEFAULT_LIGHT_COLOR = float3(1.0f, 1.0f, 1.0f);
static const float DEFAULT_LIGHT_INTENSITY = 3.0f;

// -----------------------------------------------------------------------------------
// Utility functions
// -----------------------------------------------------------------------------------
float3 safe_normalize(float3 v)
{
    float d = dot(v, v);
    if (d < 1e-8f)
        return float3(0, 0, 1);
    return v * rsqrt(d);
}

// Fresnel Schlick approximation
float3 Fresnel_Schlick(float cosTheta, float3 F0)
{
    // Schlick's approximation
    return F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0f);
}

// Normal Distribution Function (GGX / Trowbridge-Reitz)
float D_GGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0f);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom = 3.14159265f * denom * denom;
    return a2 / denom;
}

// Geometry: Schlick-GGX (single term)
float G_SchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0f);
    float k = (r * r) / 8.0f;
    return NdotV / (NdotV * (1.0f - k) + k);
}

// Geometry Smith (both view and light)
float G_Smith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0f);
    float NdotL = max(dot(N, L), 0.0f);
    float ggxV = G_SchlickGGX(NdotV, roughness);
    float ggxL = G_SchlickGGX(NdotL, roughness);
    return ggxV * ggxL;
}

float3 PBR(float3 albedo, float3 lightDirectionWS, float3 cameraPosWS, float3 posWS, float3 N)
{
    //float3 albedo = DEFAULT_ALBEDO;
    float metallic = DEFAULT_METALLIC;
    float roughness = DEFAULT_ROUGH;
    float ao = DEFAULT_AO;
    float3 lightColor = DEFAULT_LIGHT_COLOR;
    float lightIntensity = DEFAULT_LIGHT_INTENSITY;
    
    // View and light directions (world-space)
    float3 V = safe_normalize(cameraPosWS - posWS); // from surface to camera
    float3 L = safe_normalize(-lightDirectionWS); // assuming lightDirectionWS points *from light*, so invert if needed
    float3 H = safe_normalize(V + L); // half-vector

    // --- BRDF terms ---
    float3 F0 = float3(0.04f, 0.04f, 0.04f); // dielectric base reflectance
    F0 = lerp(F0, albedo, metallic); // if metallic, F0 = albedo

    float NdotL = max(dot(N, L), 0.0f);
    float NdotV = max(dot(N, V), 0.0f);

    // NDF
    float D = D_GGX(N, H, roughness);

    // Fresnel
    float3 F = Fresnel_Schlick(max(dot(H, V), 0.0f), F0);

    // Geometry
    float G = G_Smith(N, V, L, roughness);

    // Specular numerator and denominator
    float3 numerator = D * G * F;
    float denom = 4.0f * max(NdotV, 0.001f) * max(NdotL, 0.001f);
    float3 specular = numerator / denom;

    // kS is specular reflectance, kD is diffuse component (energy conservation)
    float3 kS = F;
    float3 kD = (1.0f - kS) * (1.0f - metallic);

    // Lambertian diffuse
    float3 diffuse = albedo / 3.14159265f;

    // Incoming radiance from directional light (simple)
    float3 radiance = lightColor * lightIntensity; // assume radiance already scaled (e.g., intensity)

    // Final shading
    float3 Lo = (kD * diffuse + specular) * radiance * NdotL;

    // Ambient (very simple ambient term)
    float3 ambient = float3(0.05f, 0.05f, 0.05f) * albedo * ao;

    float3 colorLinear = ambient + Lo;

    // tone mapping / gamma conversion is usually done in post; here we just output linear color
    return colorLinear;
}