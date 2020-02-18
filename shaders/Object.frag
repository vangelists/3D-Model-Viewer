#version 150 core

// SPDX-License-Identifier: MPL-2.0
// Copyright (c) 2017 Vangelis Tsiatsianas

in vec3 worldPosition;
in vec3 worldEye;
in vec3 worldNormal;
in vec2 texCoord;

out vec4 fragColor;

uniform vec4 ambientProduct;
uniform vec4 diffuseProduct;
uniform vec4 specularProduct;
uniform vec4 lightPosition;

uniform float lightIntensity;
uniform float materialShininess;

uniform sampler2D tex;
uniform uint hasTextures;

void main() {
    vec3 normal = normalize(worldNormal);
    vec3 eye = normalize(worldEye);
    vec3 light = normalize(vec3(lightPosition) - worldPosition);
    vec3 halfway = normalize(light + eye);

    vec4 ambient = ambientProduct;

    float kd = max(dot(light, normal), 0.0);
    vec4 diffuse = kd * diffuseProduct;

    float ks = pow(max(dot(normal, halfway), 0.0), materialShininess);
    vec4 specular;
    if (dot(light, normal) < 0.0) {
        specular = vec4(0.0, 0.0, 0.0, 1.0);
    } else {
        specular = ks * specularProduct;
    }

    fragColor = ambient + (lightIntensity / 100) * (diffuse + specular);
    fragColor.a = 1.0;

    if (hasTextures != uint(0)) {
        fragColor *= texture(tex, texCoord);
    }
}
