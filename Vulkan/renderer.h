// minimalistic code to draw a single triangle, this is not part of the API.
// TODO: Part 1b
#include "../Vulkan/FSLogo.h"
#include "h2bParser.h"
#include "Model.h"
#include "shaderc/shaderc.h" // needed for compiling shaders at runtime
#ifdef _WIN32 // must use MT platform DLL libraries on windows
#pragma comment(lib, "shaderc_combined.lib") 
#endif

std::string ShaderAsString(const char* shaderFilePath) {
	std::string output;
	unsigned int stringLength = 0;
	GW::SYSTEM::GFile file; file.Create();
	file.GetFileSize(shaderFilePath, stringLength);
	if (stringLength && +file.OpenBinaryRead(shaderFilePath)) {
		output.resize(stringLength);
		file.Read(&output[0], stringLength);
	}
	return output;
}

vector<Model> models = Text2Model("../DEV4Git/FoodLevel.txt");

// Simple Vertex Shader
std::string verShader = ShaderAsString("../VertexShader.hlsl");
const char* vertexShaderSource = verShader.c_str();
// Simple Pixel Shader
std::string pixShader = ShaderAsString("../PixelShader.hlsl");
const char* pixelShaderSource = pixShader.c_str();
// Creation, Rendering & Cleanup
class Renderer
{
	// TODO: Part 2b
//#define MAX_SUBMESH_PER_DRAW 1024
	//struct SHADER_MODEL_DATA
	//{
	//	GW::MATH::GVECTORF sunDirection, sunColor;
	//	GW::MATH::GMATRIXF view, projection;

	//	GW::MATH::GMATRIXF matricies[MAX_SUBMESH_PER_DRAW];
	//	OBJ_ATTRIBUTES materials[MAX_SUBMESH_PER_DRAW];
	//};

	struct MaterialData
	{
		unsigned int idx;
	};

	float deltaTime;
	std::chrono::steady_clock::time_point lastUpdate;

	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GVulkanSurface vlk;
	GW::CORE::GEventReceiver shutdown;
	GW::INPUT::GInput gInputProxy;
	GW::INPUT::GController gControllerProxy;
	GW::AUDIO::GMusic gMusic;

	// what we need at a minimum to draw a triangle
	VkDevice device = nullptr;
	VkBuffer vertexHandle = nullptr;
	VkDeviceMemory vertexData = nullptr;
	// TODO: Part 1g
	VkBuffer buffer2 = nullptr;
	VkDeviceMemory deviceMemory2 = nullptr;
	// TODO: Part 2c
	//std::vector<VkBuffer> bufferVector;
	//std::vector<VkDeviceMemory> memoryVector;
	GW::AUDIO::GAudio audio;

	VkShaderModule vertexShader = nullptr;
	VkShaderModule pixelShader = nullptr;
	// pipeline settings for drawing (also required)
	VkPipeline pipeline = nullptr;
	VkPipelineLayout pipelineLayout = nullptr;
	// TODO: Part 2e
	VkDescriptorSetLayout vkDSLayout = nullptr;
	// TODO: Part 2f
	VkDescriptorPool vkPool = nullptr;
	// TODO: Part 2g
	VkDescriptorSet vkDS = nullptr;

	// TODO: Part 4f

	vector<SHADER_MODEL_DATA> smdVec;

	// TODO: Part 2a
	GW::MATH::GMatrix mProxy;
	GW::MATH::GMATRIXF project = GW::MATH::GIdentityMatrixF;
	GW::MATH::GVECTORF eye = { 5.75f, 5.25f, -30.5f, 0 };
	GW::MATH::GVECTORF at = { 0.15f, 5.75f, 0 };
	GW::MATH::GVECTORF up = { 0, 1, 0 };
	GW::MATH::GVECTORF LightDir = { 10, -20, 10};
	GW::MATH::GVECTORF LightCol = { 1.0f, 1.0f, 1.0f, 1 };
	// TODO: Part 2b
	//SHADER_MODEL_DATA smd;
	GW::MATH::GMATRIXF cam;
			float aspect = 0.0f;
			unsigned int image = 0;

	// TODO: Part 4g
public:
	void CreateBuffer()
	{
		for (size_t i = 0; i < models.size(); i++)
		{
			mProxy.ProjectionVulkanLHF(G_DEGREE_TO_RADIAN(65), aspect, 0.1f, 100.0f, models[i].smd.projection);
			mProxy.IdentityF(models[i].smd.view);
			mProxy.LookAtLHF(eye, at, up, models[i].smd.view);
			models[i].smd.sunColor = LightCol;
			models[i].smd.sunDirection = LightDir;
			models[i].smd.sunColor = LightCol;

		}


		VkPhysicalDevice physicalDevice = nullptr;
		vlk.GetDevice((void**)&device);
		vlk.GetPhysicalDevice((void**)&physicalDevice);


		for (size_t i = 0; i < models.size(); i++)
		{
			GvkHelper::create_buffer(physicalDevice, device, sizeof(H2B::VERTEX) * models[i].parse.vertices.size(),
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &models[i].buffer, &models[i].deviceMemory);
			GvkHelper::write_to_buffer(device, models[i].deviceMemory, models[i].parse.vertices.data(), sizeof(H2B::VERTEX) * models[i].parse.vertices.size());

			GvkHelper::create_buffer(physicalDevice, device, sizeof(unsigned int) * models[i].parse.indices.size(),
				VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &models[i].indicieBuffer, &models[i].indicieMemory);
			GvkHelper::write_to_buffer(device, models[i].indicieMemory, models[i].parse.indices.data(), sizeof(unsigned int) * models[i].parse.indices.size());
		}
		// TODO: Part 1g
		/*GvkHelper::create_buffer(physicalDevice, device, sizeof(FSLogo_indices),
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &buffer2, &deviceMemory2);
		GvkHelper::write_to_buffer(device, deviceMemory2, FSLogo_indices, sizeof(FSLogo_indices));*/
		// TODO: Part 2d
		for (size_t i = 0; i < models.size(); i++)
		{
			smdVec.push_back(models[i].smd);
		}
		for (int j = 0; j < image; j++)
		{
			GvkHelper::create_buffer(physicalDevice, device, sizeof(SHADER_MODEL_DATA) * smdVec.size(),
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &bufferVector[j], &memoryVector[j]);
			GvkHelper::write_to_buffer(device, memoryVector[j], smdVec.data(), sizeof(SHADER_MODEL_DATA) * smdVec.size());
		}

	}

	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GVulkanSurface _vlk)
	{
		win = _win;
		vlk = _vlk;
		vlk.GetSwapchainImageCount(image);

		bufferVector.resize(image);
		memoryVector.resize(image);

		unsigned int width, height;
		win.GetClientWidth(width);
		win.GetClientHeight(height);
		// TODO: Part 2a
		mProxy.Create();
		gInputProxy.Create(win);
		gControllerProxy.Create();
		audio.Create();
		audio.SetMasterVolume(1.0f);
		audio.PlayMusic();
		gMusic.Create("../DEV4Git/Akatsuki.wav", audio);
		gMusic.Play(true);
		vlk.GetAspectRatio(aspect);
		// TODO: Part 2b
		for (size_t i = 0; i < models.size(); i++)
		{
			mProxy.ProjectionVulkanLHF(G_DEGREE_TO_RADIAN(65), aspect, 0.1f, 100.0f, models[i].smd.projection);
			mProxy.IdentityF(models[i].smd.view);
			mProxy.LookAtLHF(eye, at, up, models[i].smd.view);
			models[i].smd.sunColor = LightCol;
			models[i].smd.sunDirection = LightDir;
			models[i].smd.sunColor = LightCol;
			
		}

		/*models[0].smd.sunColor = LightCol;
		models[0].smd.sunDirection = LightDir;*/
		//mProxy.IdentityF(models[0].smd.matricies[1]);
		// TODO: Part 4g
		// TODO: part 3b

		/***************** GEOMETRY INTIALIZATION ******************/
		// Grab the device & physical device so we can allocate some stuff
		VkPhysicalDevice physicalDevice = nullptr;
		vlk.GetDevice((void**)&device);
		vlk.GetPhysicalDevice((void**)&physicalDevice);

		// TODO: Part 1c
		// Create Vertex Buffer
		/*float verts[(FSLogo_vertexcount * 3)] = {};
		for (size_t i = 0; i < (FSLogo_vertexcount * 3); i+=3)
		{
			verts[i] = FSLogo_vertices[i].pos.x;
			verts[i+1] = FSLogo_vertices[i].pos.y;
			verts[i+2] = FSLogo_vertices[i].pos.z;
		}*/
		// Transfer triangle data to the vertex buffer. (staging would be prefered here)
		//GvkHelper::create_buffer(physicalDevice, device, sizeof(FSLogo_vertices),
		//	VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		//	VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vertexHandle, &vertexData);
		//GvkHelper::write_to_buffer(device, vertexData, FSLogo_vertices, sizeof(FSLogo_vertices));

		for (size_t i = 0; i < models.size(); i++)
		{
			GvkHelper::create_buffer(physicalDevice, device, sizeof(H2B::VERTEX) * models[i].parse.vertices.size(),
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &models[i].buffer, &models[i].deviceMemory);
			GvkHelper::write_to_buffer(device, models[i].deviceMemory, models[i].parse.vertices.data(), sizeof(H2B::VERTEX) * models[i].parse.vertices.size());

			GvkHelper::create_buffer(physicalDevice, device, sizeof(unsigned int) * models[i].parse.indices.size(),
				VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &models[i].indicieBuffer, &models[i].indicieMemory);
			GvkHelper::write_to_buffer(device, models[i].indicieMemory, models[i].parse.indices.data(), sizeof(unsigned int) * models[i].parse.indices.size());
		}
		// TODO: Part 1g
		/*GvkHelper::create_buffer(physicalDevice, device, sizeof(FSLogo_indices),
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &buffer2, &deviceMemory2);
		GvkHelper::write_to_buffer(device, deviceMemory2, FSLogo_indices, sizeof(FSLogo_indices));*/
		// TODO: Part 2d
		for (size_t i = 0; i < models.size(); i++)
		{
			smdVec.push_back(models[i].smd);
		}
		for (int j = 0; j < image; j++)
		{
			GvkHelper::create_buffer(physicalDevice, device, sizeof(SHADER_MODEL_DATA) * smdVec.size(),
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &bufferVector[j], &memoryVector[j]);
			GvkHelper::write_to_buffer(device, memoryVector[j], smdVec.data(), sizeof(SHADER_MODEL_DATA) * smdVec.size());
		}


		//GvkHelper::create_buffer(physicalDevice, device, sizeof(Model::smd),
		//	VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		//	VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &bufferVector[i], &memoryVector[i]);
		//GvkHelper::write_to_buffer(device, memoryVector[i], &smd, sizeof(Model::smd));

		/***************** SHADER INTIALIZATION ******************/
		// Intialize runtime shader compiler HLSL -> SPIRV
		shaderc_compiler_t compiler = shaderc_compiler_initialize();
		shaderc_compile_options_t options = shaderc_compile_options_initialize();
		shaderc_compile_options_set_source_language(options, shaderc_source_language_hlsl);
		shaderc_compile_options_set_invert_y(options, false); // TODO: Part 2i
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
		// TODO: Part 1e
		// Vertex Input State
		VkVertexInputBindingDescription vertex_binding_description = {};
		vertex_binding_description.binding = 0;
		vertex_binding_description.stride = sizeof(float) * 9;
		vertex_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		VkVertexInputAttributeDescription vertex_attribute_description[3] = {
			{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0}, //uv, normal, etc....
			{ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3}, //uv, normal, etc....
			{ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 6 } //uv, normal, etc....
		};
		VkPipelineVertexInputStateCreateInfo input_vertex_info = {};
		input_vertex_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		input_vertex_info.vertexBindingDescriptionCount = 1;
		input_vertex_info.pVertexBindingDescriptions = &vertex_binding_description;
		input_vertex_info.vertexAttributeDescriptionCount = 3;
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
		rasterization_create_info.polygonMode = VK_POLYGON_MODE_FILL;
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
		depth_stencil_create_info.depthTestEnable = VK_TRUE;
		depth_stencil_create_info.depthWriteEnable = VK_TRUE;
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

		// TODO: Part 2e
		VkDescriptorSetLayoutBinding desc_layout_binding = {};
		desc_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		desc_layout_binding.binding = 0;
		desc_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;
		desc_layout_binding.descriptorCount = 1;
		desc_layout_binding.pImmutableSamplers = VK_NULL_HANDLE;
		VkDescriptorSetLayoutCreateInfo desc_layout_info = {};
		desc_layout_info.bindingCount = 1;
		desc_layout_info.pBindings = &desc_layout_binding;
		desc_layout_info.pNext = nullptr;
		desc_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

		vkCreateDescriptorSetLayout(device, &desc_layout_info, nullptr, &vkDSLayout);
		// TODO: Part 2f
		VkDescriptorPoolSize desc_pool_size = {};
		desc_pool_size.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		desc_pool_size.descriptorCount = 1;
		VkDescriptorPoolCreateInfo desc_pool_info = {};
		desc_pool_info.poolSizeCount = 1;
		desc_pool_info.pNext = nullptr;
		desc_pool_info.pPoolSizes = &desc_pool_size;
		desc_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		desc_pool_info.maxSets = 1;

		vkCreateDescriptorPool(device, &desc_pool_info, nullptr, &vkPool);
		// TODO: Part 4f
	// TODO: Part 2g
		VkDescriptorSetAllocateInfo desc_set_info = {};
		desc_set_info.descriptorPool = vkPool;
		desc_set_info.descriptorSetCount = 1;
		desc_set_info.pSetLayouts = &vkDSLayout;
		desc_set_info.pNext = nullptr;
		desc_set_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;

		vkAllocateDescriptorSets(device, &desc_set_info, &vkDS);
		// TODO: Part 4f
	// TODO: Part 2h
		VkDescriptorBufferInfo desc_buff_info = {};
		desc_buff_info.buffer = bufferVector[0];
		desc_buff_info.offset = 0;
		desc_buff_info.range = VK_WHOLE_SIZE;
		VkWriteDescriptorSet write_desc_set = {};
		write_desc_set.descriptorCount = 1;
		write_desc_set.dstArrayElement = 0;
		write_desc_set.pNext = nullptr;
		write_desc_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_desc_set.dstBinding = 0;
		write_desc_set.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		write_desc_set.dstSet = vkDS;
		write_desc_set.pBufferInfo = &desc_buff_info;

		vkUpdateDescriptorSets(device, 1, &write_desc_set, 0, nullptr);
		// TODO: Part 4f
		VkPushConstantRange constant_range_info = {};
		constant_range_info.offset = 0;
		constant_range_info.size = sizeof(unsigned int) * 2;
		constant_range_info.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		// Descriptor pipeline layout
		VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
		pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		// TODO: Part 2e
		pipeline_layout_create_info.setLayoutCount = 1;
		pipeline_layout_create_info.pSetLayouts = &vkDSLayout;
		// TODO: Part 3c
		pipeline_layout_create_info.pushConstantRangeCount = 1;
		pipeline_layout_create_info.pPushConstantRanges = &constant_range_info;
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
		// TODO: Part 2a
		//auto now = std::chrono::steady_clock::now();
		//deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(now - lastUpdate).count() / 1000000.0f; //seconds
		//lastUpdate = now;

		//mProxy.RotateYLocalF(smd.matricies[1], deltaTime, smd.matricies[1]);

		// TODO: Part 4d
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
		// TODO: Part 1h
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
			0, 1, &vkDS, 0, nullptr);

		//vkCmdDrawIndexed(commandBuffer, 8532, 1, 0, 0, 0);
		// TODO: Part 4d
		// TODO: Part 2i
		// TODO: Part 3b

		for (size_t i = 0; i < smdVec.size(); i++)
		{
			smdVec[i].view = models[i].smd.view;
		}
		GvkHelper::write_to_buffer(device, memoryVector[0], smdVec.data(), sizeof(SHADER_MODEL_DATA) * smdVec.size());

		for (int i = 0; i < models.size(); i++)
		{
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, &models[i].buffer, offsets);
			vkCmdBindIndexBuffer(commandBuffer, models[i].indicieBuffer, 0, VkIndexType::VK_INDEX_TYPE_UINT32);

			for (int j = 0; j < models[i].parse.meshes.size(); j++)
			{
				unsigned int ID[2] = { i, j }; //models[i].parse.meshes[j].materialIndex
				vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
					0, sizeof(unsigned int) * 2, ID);
				vkCmdDrawIndexed(commandBuffer, models[i].parse.meshes[j].drawInfo.indexCount, 1,
					models[i].parse.meshes[j].drawInfo.indexOffset, 0, 0);
			}
		}

		/*vkCmdBindVertexBuffers(commandBuffer, 0, 1, &models[6].buffer, offsets);
		vkCmdBindIndexBuffer(commandBuffer, models[6].indicieBuffer, 0, VkIndexType::VK_INDEX_TYPE_UINT32);
		GvkHelper::write_to_buffer(device, memoryVector[0], &models[6].smd, sizeof(SHADER_MODEL_DATA));

		for (size_t i = 0; i < 1; i++)
		{
			vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0, sizeof(unsigned int), &models[6].parse.meshes[i].materialIndex);
			vkCmdDrawIndexed(commandBuffer, models[6].parse.meshes[i].drawInfo.indexCount, 1,
				models[6].parse.meshes[i].drawInfo.indexOffset, 0, 0);
		}*/
		// TODO: Part 3d
	//vkCmdDraw(commandBuffer, 3885, 1, 0, 0); // TODO: Part 1d, 1h

	}

	void UpdateCamera()
	{
		GW::MATH::GMATRIXF cam = GW::MATH::GIdentityMatrixF;

		auto now = std::chrono::steady_clock::now();
		deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(now - lastUpdate).count() / 1000000.0f; //seconds
		lastUpdate = now;

		// TODO: Part 4c
		mProxy.InverseF(models[0].smd.view, cam);
		// TODO: Part 4d
		float y = 0.0f;

		float totalY = 0.0f;
		float totalZ = 0.0f;
		float totalX = 0.0f;

		const float Camera_Speed = 5.0f;
		float spaceKeyState = 0.0f;
		float leftShiftState = 0.0f;
		float rightTriggerState = 0.0f;
		float leftTriggerState = 0.0f;

		float arrowRight = 0.0f;
		float arrowLeft = 0.0f;

		float wKeyState = 0.0f;
		float sKeyState = 0.0f;
		float aKeyState = 0.0f;
		float dKeyState = 0.0f;
		float leftStickX = 0.0f;
		float leftStickY = 0.0f;
		unsigned int screenHeight = 0.0f;
		win.GetHeight(screenHeight);
		unsigned int screenWidth = 0.0f;
		win.GetWidth(screenWidth);
		float mouseDeltaX = 0.0f;
		float mouseDeltaY = 0.0f;
		GW::GReturn result = gInputProxy.GetMouseDelta(mouseDeltaX, mouseDeltaY);
		float rightStickYaxis = 0.0f;
		gControllerProxy.GetState(0, G_RY_AXIS, rightStickYaxis);
		float rightStickXaxis = 0.0f;
		gControllerProxy.GetState(0, G_RX_AXIS, rightStickXaxis);

		float perFrameSpeed = 0.0f;
		
		gInputProxy.GetState(G_KEY_RIGHT, arrowRight);
		gInputProxy.GetState(G_KEY_LEFT, arrowLeft);

		if (arrowRight != 0)
		{
			cam.row4 = { 0.0f, 50.0f, 0.0f, 1 };

		}
		if (arrowLeft != 0)
		{
			cam.row4 = { 5.75f, 5.25f, -30.5f, 1 };
		}

		if (G_PASS(gInputProxy.GetState(G_KEY_SPACE, spaceKeyState)) &&
			spaceKeyState != 0 ||

			G_PASS(gInputProxy.GetState(G_KEY_LEFTSHIFT, leftShiftState)) &&
			leftShiftState != 0 ||

			G_PASS(gControllerProxy.GetState(0, G_RIGHT_TRIGGER_AXIS, rightTriggerState)) &&
			rightTriggerState != 0 ||

			G_PASS(gControllerProxy.GetState(0, G_LEFT_TRIGGER_AXIS, leftTriggerState)) &&
			leftTriggerState != 0)
		{
			totalY = spaceKeyState - leftShiftState + rightTriggerState - leftTriggerState;
		}

		cam.row4.y += totalY * Camera_Speed * deltaTime;

		// TODO: Part 4e
		perFrameSpeed = Camera_Speed * deltaTime;

		if (G_PASS(gInputProxy.GetState(G_KEY_W, wKeyState)) &&
			wKeyState != 0 ||

			G_PASS(gInputProxy.GetState(G_KEY_A, aKeyState)) &&
			aKeyState != 0 ||

			G_PASS(gInputProxy.GetState(G_KEY_S, sKeyState)) &&
			sKeyState != 0 ||

			G_PASS(gInputProxy.GetState(G_KEY_D, dKeyState)) &&
			dKeyState != 0 ||

			G_PASS(gControllerProxy.GetState(0, G_LX_AXIS, leftStickX)) &&
			leftStickX != 0 ||

			G_PASS(gControllerProxy.GetState(0, G_LY_AXIS, leftStickY)) &&
			leftStickY != 0)
		{
			totalZ = wKeyState - sKeyState + leftStickY;
			totalX = dKeyState - aKeyState + leftStickX;
		}

		GW::MATH::GMATRIXF translation = GW::MATH::GIdentityMatrixF;
		GW::MATH::GVECTORF vec = { totalX * perFrameSpeed, 0, totalZ * perFrameSpeed };
		mProxy.TranslateLocalF(translation, vec, translation);
		mProxy.MultiplyMatrixF(translation, cam, cam);
		// TODO: Part 4f
		float thumbSpeed = 3.14 * perFrameSpeed;
		if (G_PASS(result) && result != GW::GReturn::REDUNDANT)
		{
			float totalPitch = G_DEGREE_TO_RADIAN(65) * mouseDeltaY / screenHeight + rightStickYaxis * -thumbSpeed;
			//GW::MATH::GMATRIXF pitchMatrix = GW::MATH::GIdentityMatrixF;
			mProxy.RotateXLocalF(cam, totalPitch, cam);
			//mProxy.RotationYawPitchRollF(0, totalPitch, 0, pitchMatrix);
			//mProxy.MultiplyMatrixF(pitchMatrix, cam, cam);
			// TODO: Part 4g
			float totalYaw = G_DEGREE_TO_RADIAN(65) * aspect * mouseDeltaX / screenWidth + rightStickXaxis * thumbSpeed;
			GW::MATH::GMATRIXF yawMatrix = GW::MATH::GIdentityMatrixF;
			//mProxy.RotationYawPitchRollF(totalYaw, 0, 0, yawMatrix);
			GW::MATH::GVECTORF camSave = cam.row4;
			cam.row4 = { 0,0,0,1 };
			mProxy.RotateYGlobalF(cam, totalYaw, cam);
			cam.row4 = camSave;
		}
	//	mProxy.MultiplyMatrixF(yawMatrix, cam, cam);
		// TODO: Part 4c
		mProxy.InverseF(cam, models[0].smd.view);
		for (size_t i = 1; i < models.size(); i++)
		{
			models[i].smd.view = models[0].smd.view;
		}
	}


private:
	void CleanUp()
	{
		// wait till everything has completed
		vkDeviceWaitIdle(device);
		// Release allocated buffers, shaders & pipeline
		// TODO: Part 1g
		vkDestroyBuffer(device, buffer2, nullptr);
		vkFreeMemory(device, deviceMemory2, nullptr);
		// TODO: Part 2d
		for (int i = 0; i < bufferVector.size(); i++)
		{
			vkDestroyBuffer(device, bufferVector[i], nullptr);
			vkFreeMemory(device, memoryVector[i], nullptr);
		}
		for (size_t i = 0; i < models.size(); i++)
		{
			vkDestroyBuffer(device, models[i].buffer, nullptr);
			vkFreeMemory(device, models[i].deviceMemory, nullptr);
			vkDestroyBuffer(device, models[i].indicieBuffer, nullptr);
			vkFreeMemory(device, models[i].indicieMemory, nullptr);
		}

		vkDestroyBuffer(device, vertexHandle, nullptr);
		vkFreeMemory(device, vertexData, nullptr);
		vkDestroyShaderModule(device, vertexShader, nullptr);
		vkDestroyShaderModule(device, pixelShader, nullptr);
		// TODO: Part 2e
		vkDestroyDescriptorSetLayout(device, vkDSLayout, nullptr);
		// TODO: part 2f
		vkDestroyDescriptorPool(device, vkPool, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyPipeline(device, pipeline, nullptr);
	}
};
