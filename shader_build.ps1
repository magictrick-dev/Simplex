slangc ./resources/shaders/scratch/core.slang   `
    -target spirv                               `
    -profile spirv_1_4                          `
    -emit-spirv-directly                        `
    -fvk-use-entrypoint-name                    `
    -entry vertex_main                          `
    -entry fragment_main                        `
    -o ./resources/shaders/scratch/core.spv