// minimalistic code to draw a single triangle, this is not part of the API.
#include "shaderc/shaderc.h" // needed for compiling shaders at runtime
#ifdef _WIN32 // must use MT platform DLL libraries on windows
#pragma comment(lib, "shaderc_combined.lib") 
#endif

#define STAR_COUNT 20000

float RandomFloat(float min, float max)
{
	float ratio = rand() / (float)RAND_MAX;
	return (max - min) * ratio + min;
}

// TODO: Part 2b
// Simple Vertex Shader
const char* vertexShaderSource = R"(
// an ultra simple hlsl vertex shader
float4 main(float2 inputVertex : POSITION) : SV_POSITION
{
	return float4(inputVertex, 0, 1);
}
)";
// Simple Pixel Shader
const char* pixelShaderSource = R"(
// an ultra simple hlsl pixel shader
float4 main() : SV_TARGET 
{	
	return float4(1.0f ,1.0f, 1.0f, 0); // TODO: Part 1a 
}
)";
// TODO: Part 4b
const char* vertexShaderSource2 = R"(
// TODO: Part 4c (in new shader)
struct Vertex
{
	float2 pos : POSITION;
	float4 clr : COLOR;
};

// TODO: Part 4d (in new shader)
struct VERTEX_OUT
{
	float4 pos : SV_POSITION; //SV = System Value
	float4 clr : COLOR;
};

// TODO: Part 4e

// Vertex Shader
VERTEX_OUT main(Vertex _vertex) : SV_POSITION
{
	VERTEX_OUT tempOUT;
	tempOUT.pos = float4(_vertex.pos[0],_vertex.pos[1], 0, 1);
	tempOUT.clr = float4(_vertex.clr);
	return tempOUT;
}
)";
const char* pixelShaderSource2 = R"(
//Pixel Shader
struct VERTEX_OUT
{
	float4 pos : SV_POSITION; //SV = System Value
	float4 clr : COLOR;
};

float4 main(VERTEX_OUT _outVert) : SV_TARGET 
{	
	return float4(_outVert.clr); 
	//return float4(1.0f, 0.0f,0.0f, 1.0f); 
}
)";
// Creation, Rendering & Cleanup
class Renderer
{
	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GVulkanSurface vlk;
	GW::CORE::GEventReceiver shutdown;
	// what we need at a minimum to draw a triangle
	VkDevice device = nullptr;
	VkBuffer vertexHandle = nullptr;
	VkDeviceMemory vertexData = nullptr;
	// TODO: Part 3a
	VkBuffer vertexBuff2 = nullptr;
	VkDeviceMemory vertDev2 = nullptr;
	VkShaderModule vertexShader = nullptr;
	VkShaderModule pixelShader = nullptr;
	// TODO: Part 4b
	VkShaderModule vertexShader2 = nullptr;
	VkShaderModule pixelShader2 = nullptr;

	// pipeline settings for drawing (also required)
	VkPipeline pipeline = nullptr;
	VkPipelineLayout pipelineLayout = nullptr;

	VkPipeline starPipeline = nullptr;
	VkPipelineLayout starPipelineLayout = nullptr;
public:
	// TODO: Part 4a
	struct Vertex
	{
		float pos[2];
		float RGBA[4];
	};

	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GVulkanSurface _vlk)
	{
		win = _win;
		vlk = _vlk;
		unsigned int width, height;
		win.GetClientWidth(width);
		win.GetClientHeight(height);
		/***************** GEOMETRY INTIALIZATION ******************/
		// Grab the device & physical device so we can allocate some stuff
		VkPhysicalDevice physicalDevice = nullptr;
		vlk.GetDevice((void**)&device);
		vlk.GetPhysicalDevice((void**)&physicalDevice);
		// TODO: Part 2b
		// Create Vertex Buffer
		float verts[STAR_COUNT]; /*= {
			   0,   0.5f,
			 0.5f, -0.5f,
			-0.5f, -0.5f
		};*/
		for (size_t i = 0; i < STAR_COUNT; i++)
		{
			verts[i] = RandomFloat(-1, 1);
		}
		// Transfer triangle data to the vertex buffer. (staging would be prefered here)
		GvkHelper::create_buffer(physicalDevice, device, sizeof(verts),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vertexHandle, &vertexData);
		GvkHelper::write_to_buffer(device, vertexData, verts, sizeof(verts));
		// TODO: Part 3a
		float star[] = {
	0.0f, 0.99f, // top // 
	-0.25f, 0.25f, //midLeft-Top // 
	-0.99f, 0.0f, // left //
	-0.375f, -0.25, //midBottomLeft-Left //
	-0.5f, -0.99f, // bottomLeft //
	0.0, -0.5, // midBottomRight-bottomLeft //
	0.5f, -0.99f, // bottomRight //
	0.375f, -0.25, //midRight-bottomRight //
	0.99f, 0.0f, // right //
	0.25f, 0.25f, //midTop-Right //
		0.0f, 0.99f, // top

		};

	/*	GvkHelper::create_buffer(physicalDevice, device, sizeof(star),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vertexBuff2, &vertDev2);
		GvkHelper::write_to_buffer(device, vertDev2, star, sizeof(star));*/
		// TODO: Part 4a
		/*STAR POINTS*/
		Vertex top =
		{
			{0.0f, 0.99f},
			{rand() / static_cast<float>(RAND_MAX),
			rand() / static_cast<float>(RAND_MAX),
			rand() / static_cast<float>(RAND_MAX),
			rand() / static_cast<float>(RAND_MAX)}
		};
		Vertex midLeftTop =
		{
			{-0.25f, 0.25f},
			{rand() / static_cast<float>(RAND_MAX),
			rand() / static_cast<float>(RAND_MAX),
			rand() / static_cast<float>(RAND_MAX),
			rand() / static_cast<float>(RAND_MAX)}
		};
		Vertex left =
		{
			{-0.99f, 0.0f},
			{rand() / static_cast<float>(RAND_MAX),
			rand() / static_cast<float>(RAND_MAX),
			rand() / static_cast<float>(RAND_MAX),
			rand() / static_cast<float>(RAND_MAX)}
		};
		Vertex midBleftLeft =
		{
			{-0.375f, -0.25},
			{rand() / static_cast<float>(RAND_MAX),
			rand() / static_cast<float>(RAND_MAX),
			rand() / static_cast<float>(RAND_MAX),
			rand() / static_cast<float>(RAND_MAX)}
		};
		Vertex bLeft =
		{
			{-0.5f, -0.99f},
			{rand() / static_cast<float>(RAND_MAX),
			rand() / static_cast<float>(RAND_MAX),
			rand() / static_cast<float>(RAND_MAX),
			rand() / static_cast<float>(RAND_MAX)}
		};
		Vertex midBrightBleft =
		{
			{0.0, -0.5},
			{rand() / static_cast<float>(RAND_MAX),
			rand() / static_cast<float>(RAND_MAX),
			rand() / static_cast<float>(RAND_MAX),
			rand() / static_cast<float>(RAND_MAX)}
		};
		Vertex bRight =
		{
			{0.5f, -0.99f},
			{rand() / static_cast<float>(RAND_MAX),
			rand() / static_cast<float>(RAND_MAX),
			rand() / static_cast<float>(RAND_MAX),
			rand() / static_cast<float>(RAND_MAX)}
		};
		Vertex midBrightRight =
		{
			{0.375f, -0.25},
			{rand() / static_cast<float>(RAND_MAX),
			rand() / static_cast<float>(RAND_MAX),
			rand() / static_cast<float>(RAND_MAX),
			rand() / static_cast<float>(RAND_MAX)}
		};
		Vertex right =
		{
			{0.99f, 0.0f},
			{rand() / static_cast<float>(RAND_MAX),
			rand() / static_cast<float>(RAND_MAX),
			rand() / static_cast<float>(RAND_MAX),
			rand() / static_cast<float>(RAND_MAX)}
		};
		Vertex midRightTop =
		{
			{0.25f, 0.25f},
			{rand() / static_cast<float>(RAND_MAX),
			rand() / static_cast<float>(RAND_MAX),
			rand() / static_cast<float>(RAND_MAX),
			rand() / static_cast<float>(RAND_MAX)}
		};

		Vertex vertArr[] = {
			top, midLeftTop, left,
			midBleftLeft, bLeft, midBrightBleft,
			bRight, midBrightRight, right,
			midRightTop, top
		};

		GvkHelper::create_buffer(physicalDevice, device, sizeof(vertArr),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vertexBuff2, &vertDev2);
		GvkHelper::write_to_buffer(device, vertDev2, vertArr, sizeof(vertArr));
		/***************** SHADER INTIALIZATION ******************/
		// Intialize runtime shader compiler HLSL -> SPIRV
		shaderc_compiler_t compiler = shaderc_compiler_initialize();
		shaderc_compile_options_t options = shaderc_compile_options_initialize();
		shaderc_compile_options_set_source_language(options, shaderc_source_language_hlsl);
		shaderc_compile_options_set_invert_y(options, true);
#ifndef NDEBUG
		shaderc_compile_options_set_generate_debug_info(options);
#endif
		// Create Vertex Shader
		shaderc_compilation_result_t result = shaderc_compile_into_spv( // compile
			compiler, vertexShaderSource, strlen(vertexShaderSource),
			shaderc_vertex_shader, "main.vert", "main", options);
		if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) // errors?
			std::cout << "Vertex Shader Errors: " << shaderc_result_get_error_message(result) << std::endl;
		GvkHelper::create_shader_module(device, shaderc_result_get_length(result), // load into Vulkan
			(char*)shaderc_result_get_bytes(result), &vertexShader);
		shaderc_result_release(result); // done
		// Create Pixel Shader
		result = shaderc_compile_into_spv( // compile
			compiler, pixelShaderSource, strlen(pixelShaderSource),
			shaderc_fragment_shader, "main.frag", "main", options);
		if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) // errors?
			std::cout << "Pixel Shader Errors: " << shaderc_result_get_error_message(result) << std::endl;
		GvkHelper::create_shader_module(device, shaderc_result_get_length(result), // load into Vulkan
			(char*)shaderc_result_get_bytes(result), &pixelShader);
		shaderc_result_release(result); // done

		// TODO: Part 4b
		// Create Vertex Shader
		 result = shaderc_compile_into_spv( // compile
			compiler, vertexShaderSource2, strlen(vertexShaderSource2),
			shaderc_vertex_shader, "main.vert", "main", options);
		if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) // errors?
			std::cout << "Vertex Shader Errors: " << shaderc_result_get_error_message(result) << std::endl;
		GvkHelper::create_shader_module(device, shaderc_result_get_length(result), // load into Vulkan
			(char*)shaderc_result_get_bytes(result), &vertexShader2);
		shaderc_result_release(result); // done
		//// Create Pixel Shader
		result = shaderc_compile_into_spv( // compile
			compiler, pixelShaderSource2, strlen(pixelShaderSource2),
			shaderc_fragment_shader, "main.frag", "main", options);
		if (shaderc_result_get_compilation_status(result) != shaderc_compilation_status_success) // errors?
			std::cout << "Pixel Shader Errors: " << shaderc_result_get_error_message(result) << std::endl;
		GvkHelper::create_shader_module(device, shaderc_result_get_length(result), // load into Vulkan
			(char*)shaderc_result_get_bytes(result), &pixelShader2);
		shaderc_result_release(result); // done
		// Free runtime shader compiler resources
		shaderc_compile_options_release(options);
		shaderc_compiler_release(compiler);

		/***************** PIPELINE INTIALIZATION ******************/
		// Create Pipeline & Layout (Thanks Tiny!)
		VkRenderPass renderPass;
		vlk.GetRenderPass((void**)&renderPass);
		VkPipelineShaderStageCreateInfo stage_create_info[2] = {};
		// Create Stage Info for Vertex Shader
		stage_create_info[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stage_create_info[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		stage_create_info[0].module = vertexShader;
		stage_create_info[0].pName = "main";
		// Create Stage Info for Fragment Shader
		stage_create_info[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stage_create_info[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		stage_create_info[1].module = pixelShader;
		stage_create_info[1].pName = "main";

		// Assembly State
		VkPipelineInputAssemblyStateCreateInfo assembly_create_info = {};
		assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		assembly_create_info.primitiveRestartEnable = false;
		// Vertex Input State
		VkVertexInputBindingDescription vertex_binding_description = {};
		vertex_binding_description.binding = 0;
		vertex_binding_description.stride = sizeof(float) * 2;
		vertex_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VkVertexInputAttributeDescription vertex_attribute_description[1] = {
			{ 0, 0, VK_FORMAT_R32G32_SFLOAT, 0 } //uv, normal, etc....
		};

		VkPipelineVertexInputStateCreateInfo input_vertex_info = {};
		input_vertex_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		input_vertex_info.vertexBindingDescriptionCount = 1;
		input_vertex_info.pVertexBindingDescriptions = &vertex_binding_description;
		input_vertex_info.vertexAttributeDescriptionCount = 1;
		input_vertex_info.pVertexAttributeDescriptions = vertex_attribute_description;



		// Viewport State (we still need to set this up even though we will overwrite the values)
		VkViewport viewport = {
			0, 0, static_cast<float>(width), static_cast<float>(height), 0, 1
		};
		VkRect2D scissor = { {0, 0}, {width, height} };
		VkPipelineViewportStateCreateInfo viewport_create_info = {};
		viewport_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_create_info.viewportCount = 1;
		viewport_create_info.pViewports = &viewport;
		viewport_create_info.scissorCount = 1;
		viewport_create_info.pScissors = &scissor;
		// Rasterizer State
		VkPipelineRasterizationStateCreateInfo rasterization_create_info = {};
		rasterization_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterization_create_info.rasterizerDiscardEnable = VK_FALSE;
		rasterization_create_info.polygonMode = VK_POLYGON_MODE_POINT; // TODO: Part 2a
		rasterization_create_info.lineWidth = 1.0f;
		rasterization_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterization_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterization_create_info.depthClampEnable = VK_FALSE;
		rasterization_create_info.depthBiasEnable = VK_FALSE;
		rasterization_create_info.depthBiasClamp = 0.0f;
		rasterization_create_info.depthBiasConstantFactor = 0.0f;
		rasterization_create_info.depthBiasSlopeFactor = 0.0f;
		// Multisampling State
		VkPipelineMultisampleStateCreateInfo multisample_create_info = {};
		multisample_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample_create_info.sampleShadingEnable = VK_FALSE;
		multisample_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisample_create_info.minSampleShading = 1.0f;
		multisample_create_info.pSampleMask = VK_NULL_HANDLE;
		multisample_create_info.alphaToCoverageEnable = VK_FALSE;
		multisample_create_info.alphaToOneEnable = VK_FALSE;
		// Depth-Stencil State
		VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info = {};
		depth_stencil_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil_create_info.depthTestEnable = VK_FALSE;
		depth_stencil_create_info.depthWriteEnable = VK_FALSE;
		depth_stencil_create_info.depthCompareOp = VK_COMPARE_OP_LESS;
		depth_stencil_create_info.depthBoundsTestEnable = VK_FALSE;
		depth_stencil_create_info.minDepthBounds = 0.0f;
		depth_stencil_create_info.maxDepthBounds = 1.0f;
		depth_stencil_create_info.stencilTestEnable = VK_FALSE;
		// Color Blending Attachment & State
		VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
		color_blend_attachment_state.colorWriteMask = 0xF;
		color_blend_attachment_state.blendEnable = VK_FALSE;
		color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
		color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
		color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
		color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
		VkPipelineColorBlendStateCreateInfo color_blend_create_info = {};
		color_blend_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blend_create_info.logicOpEnable = VK_FALSE;
		color_blend_create_info.logicOp = VK_LOGIC_OP_COPY;
		color_blend_create_info.attachmentCount = 1;
		color_blend_create_info.pAttachments = &color_blend_attachment_state;
		color_blend_create_info.blendConstants[0] = 0.0f;
		color_blend_create_info.blendConstants[1] = 0.0f;
		color_blend_create_info.blendConstants[2] = 0.0f;
		color_blend_create_info.blendConstants[3] = 0.0f;
		// Dynamic State 
		VkDynamicState dynamic_state[2] = {
			// By setting these we do not need to re-create the pipeline on Resize
			VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamic_create_info = {};
		dynamic_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_create_info.dynamicStateCount = 2;
		dynamic_create_info.pDynamicStates = dynamic_state;
		// Descriptor pipeline layout
		VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
		pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_create_info.setLayoutCount = 0;
		pipeline_layout_create_info.pSetLayouts = VK_NULL_HANDLE;
		pipeline_layout_create_info.pushConstantRangeCount = 0;
		pipeline_layout_create_info.pPushConstantRanges = nullptr;
		vkCreatePipelineLayout(device, &pipeline_layout_create_info,
			nullptr, &pipelineLayout);
		// Pipeline State... (FINALLY) 
		VkGraphicsPipelineCreateInfo pipeline_create_info = {};
		pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_create_info.stageCount = 2;
		pipeline_create_info.pStages = stage_create_info;
		pipeline_create_info.pInputAssemblyState = &assembly_create_info;
		pipeline_create_info.pVertexInputState = &input_vertex_info;
		pipeline_create_info.pViewportState = &viewport_create_info;
		pipeline_create_info.pRasterizationState = &rasterization_create_info;
		pipeline_create_info.pMultisampleState = &multisample_create_info;
		pipeline_create_info.pDepthStencilState = &depth_stencil_create_info;
		pipeline_create_info.pColorBlendState = &color_blend_create_info;
		pipeline_create_info.pDynamicState = &dynamic_create_info;
		pipeline_create_info.layout = pipelineLayout;
		pipeline_create_info.renderPass = renderPass;
		pipeline_create_info.subpass = 0;
		pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;

		vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
			&pipeline_create_info, nullptr, &pipeline);


		// TODO: Part 3b
		VkPipelineShaderStageCreateInfo stage_create_info2[2] = {};
		// Create Stage Info for Vertex Shader
		stage_create_info2[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stage_create_info2[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		stage_create_info2[0].module = vertexShader2;
		stage_create_info2[0].pName = "main";
		// Create Stage Info for Fragment Shader
		stage_create_info2[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stage_create_info2[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		stage_create_info2[1].module = pixelShader2;
		stage_create_info2[1].pName = "main";

		//		// Vertex Input State COPY
		VkVertexInputBindingDescription vertex_binding_description2 = {};
		vertex_binding_description2.binding = 0;
		vertex_binding_description2.stride = sizeof(float) * 6;
		vertex_binding_description2.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VkVertexInputAttributeDescription vertex_attribute_description2[2] = {
			{ 0, 0, VK_FORMAT_R32G32_SFLOAT, 0 }, //uv, normal, etc....
			{ 1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 2}
		};

		VkPipelineInputAssemblyStateCreateInfo assembly_create_info_star = {};
		assembly_create_info_star.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		assembly_create_info_star.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
		assembly_create_info_star.primitiveRestartEnable = false;

		VkPipelineVertexInputStateCreateInfo input_vertex_info2 = {};
		input_vertex_info2.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		input_vertex_info2.vertexBindingDescriptionCount = 1;
		input_vertex_info2.pVertexBindingDescriptions = &vertex_binding_description2;
		input_vertex_info2.vertexAttributeDescriptionCount = 2;
		input_vertex_info2.pVertexAttributeDescriptions = vertex_attribute_description2;

		VkGraphicsPipelineCreateInfo pipeline_create_info_star = {};
		pipeline_create_info_star.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_create_info_star.stageCount = 2;
		pipeline_create_info_star.pStages = stage_create_info2;
		pipeline_create_info_star.pInputAssemblyState = &assembly_create_info_star;
		pipeline_create_info_star.pVertexInputState = &input_vertex_info2;
		pipeline_create_info_star.pViewportState = &viewport_create_info;
		pipeline_create_info_star.pRasterizationState = &rasterization_create_info;
		pipeline_create_info_star.pMultisampleState = &multisample_create_info;
		pipeline_create_info_star.pDepthStencilState = &depth_stencil_create_info;
		pipeline_create_info_star.pColorBlendState = &color_blend_create_info;
		pipeline_create_info_star.pDynamicState = &dynamic_create_info;
		pipeline_create_info_star.layout = pipelineLayout;
		pipeline_create_info_star.renderPass = renderPass;
		pipeline_create_info_star.subpass = 0;
		pipeline_create_info_star.basePipelineHandle = VK_NULL_HANDLE;

				vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
			&pipeline_create_info_star, nullptr, &starPipeline);

		// TODO: Part 4f
		/***************** CLEANUP / SHUTDOWN ******************/
		// GVulkanSurface will inform us when to release any allocated resources
		shutdown.Create(vlk, [&]() {
			if (+shutdown.Find(GW::GRAPHICS::GVulkanSurface::Events::RELEASE_RESOURCES, true)) {
				CleanUp(); // unlike D3D we must be careful about destroy timing
			}
			});
	}
	void Render()
	{
		// grab the current Vulkan commandBuffer
		unsigned int currentBuffer;
		vlk.GetSwapchainCurrentImage(currentBuffer);
		VkCommandBuffer commandBuffer;
		vlk.GetCommandBuffer(currentBuffer, (void**)&commandBuffer);
		// what is the current client area dimensions?
		unsigned int width, height;
		win.GetClientWidth(width);
		win.GetClientHeight(height);
		// setup the pipeline's dynamic settings
		VkViewport viewport = {
			0, 0, static_cast<float>(width), static_cast<float>(height), 0, 1
		};
		VkRect2D scissor = { {0, 0}, {width, height} };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		// now we can draw
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexHandle, offsets);
		vkCmdDraw(commandBuffer, 10000, 1, 0, 0); // TODO: Part 2b
		// TODO: Part 3b
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, starPipeline);

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuff2, offsets);
		vkCmdDraw(commandBuffer, 11, 1, 0, 0); // TODO: Part 2b

	}
private:
	void CleanUp()
	{
		// wait till everything has completed
		vkDeviceWaitIdle(device);
		// Release allocated buffers, shaders & pipeline
		vkDestroyBuffer(device, vertexHandle, nullptr);
		vkFreeMemory(device, vertexData, nullptr);
		// TODO: Part 3a
		vkDestroyBuffer(device, vertexBuff2, nullptr);
		vkDestroyShaderModule(device, vertexShader, nullptr);
		vkDestroyShaderModule(device, pixelShader, nullptr);
		// TODO: Part 4b
		vkDestroyShaderModule(device, vertexShader2, nullptr);
		vkDestroyShaderModule(device, pixelShader2, nullptr);

		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyPipeline(device, pipeline, nullptr);
	}
};
