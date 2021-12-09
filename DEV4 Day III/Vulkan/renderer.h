// minimalistic code to draw a single triangle, this is not part of the API.
#include "shaderc/shaderc.h" // needed for compiling shaders at runtime
#ifdef _WIN32 // must use MT platform DLL libraries on windows
#pragma comment(lib, "shaderc_combined.lib") 
#endif
using namespace std;

string ShaderAsString(const char* shaderFilePath) {
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
string vertShader = ShaderAsString("../VertexShader.hlsl");
string pixShader = ShaderAsString("../PixelShader.hlsl");
// Simple Vertex Shader
const char* vertexShaderSource = R"(
#pragma pack_matrix(row_major)
[[vk::push_constant]]
cbuffer SHADER_VARS
{
	matrix wm; //64 bytes
	matrix view;
};

// TODO: Part 2f, Part 3b
// TODO: Part 1c
struct Vertex
{
	float4 pos : POSITION;
};

float4 main(Vertex inputVertex) : SV_POSITION
{
	// TODO: Part 2d
	inputVertex.pos = mul(inputVertex.pos, wm);
	inputVertex.pos = mul(inputVertex.pos, view);
	// TODO: Part 2f, Part 3b
	return inputVertex.pos;
}
)";
const char* pixelShaderSource = pixShader.c_str();
// Creation, Rendering & Cleanup
struct Vertex
{
	float x, y, z, w;
};
// TODO: Part 2b
struct WM
{
	GW::MATH::GMATRIXF wm;
	GW::MATH::GMATRIXF view;
};

class Renderer
{
	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GVulkanSurface vlk;
	GW::CORE::GEventReceiver shutdown;
	// TODO: Part 4a
	float deltaTime;
	std::chrono::steady_clock::time_point lastUpdate;
	GW::INPUT::GInput gInputProxy;
	GW::INPUT::GController gControllerProxy;

	GW::MATH::GVECTORF eye = { 0.25f, -0.125f, -0.25f, 1 };
	GW::MATH::GVECTORF at = { 0,-0.5f,0, 1 };
	GW::MATH::GVECTORF up = { 0,1,0, 1 };

	float aspect = 0.0f;

	// TODO: Part 2a
	WM matrixBuffer;
	WM walls[5] = {};
	//GW::MATH::GMATRIXF gMatrix = GW::MATH::GIdentityMatrixF;
	GW::MATH::GMatrix mProxy;
	// TODO: Part 3d
	// TODO: Part 2e
	GW::MATH::GMATRIXF view = GW::MATH::GIdentityMatrixF;
	// TODO: Part 3a
	GW::MATH::GMATRIXF perspective = GW::MATH::GIdentityMatrixF;
	// what we need at a minimum to draw a triangle
	VkDevice device = nullptr;
	VkBuffer vertexHandle = nullptr;
	VkDeviceMemory vertexData = nullptr;
	VkShaderModule vertexShader = nullptr;
	VkShaderModule pixelShader = nullptr;
	// pipeline settings for drawing (also required)
	VkPipeline pipeline = nullptr;
	VkPipelineLayout pipelineLayout = nullptr;
public:
	// TODO: Part 1c
		// TODO: Part 2f
	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GVulkanSurface _vlk)
	{
		win = _win;
		vlk = _vlk;
		unsigned int width, height;
		win.GetClientWidth(width);
		win.GetClientHeight(height);
		// TODO: Part 4a
		gInputProxy.Create(win);
		gControllerProxy.Create();
		// TODO: Part 2a
		mProxy.Create();
		// Floor
		mProxy.IdentityF(matrixBuffer.wm);
		mProxy.IdentityF(matrixBuffer.view);

		// Walls
		for (size_t i = 0; i < 5; i++)
		{
			mProxy.IdentityF(walls[i].wm);
		}

		float aspect = 0.0f;
		vlk.GetAspectRatio(aspect);
		mProxy.ProjectionVulkanLHF(G_DEGREE_TO_RADIAN(65.0f), aspect, 0.1f, 100, perspective);
		// Floor Rotation
		mProxy.RotateXLocalF(matrixBuffer.wm, G_DEGREE_TO_RADIAN(90), matrixBuffer.wm);
		// Floor Translate
		GW::MATH::GVECTORF gVec = { 0, -0.5, 0, 1 };
		mProxy.TranslateGlobalF(matrixBuffer.wm, gVec, matrixBuffer.wm);

		// Wall Rotations
		mProxy.RotateYLocalF(walls[0].wm, G_DEGREE_TO_RADIAN(90), walls[0].wm);
		gVec = { -0.5f, 0, 0, 1 };
		mProxy.TranslateGlobalF(walls[0].wm, gVec, walls[0].wm);
		gVec = { 0.5f, 0, 0, 1 };
		mProxy.RotateYLocalF(walls[1].wm, G_DEGREE_TO_RADIAN(90), walls[1].wm);
		mProxy.TranslateGlobalF(walls[1].wm, gVec, walls[1].wm);
		gVec = { 0, 0, -0.5f, 1 };
		mProxy.TranslateGlobalF(walls[2].wm, gVec, walls[2].wm);
		gVec = { 0, 0, 0.5f, 1 };
		mProxy.TranslateGlobalF(walls[3].wm, gVec, walls[3].wm);

		mProxy.RotateXLocalF(walls[4].wm, G_DEGREE_TO_RADIAN(90), walls[4].wm);
		gVec = { 0, 0.5, 0, 1 };
		mProxy.TranslateGlobalF(walls[4].wm, gVec, walls[4].wm);

		// TODO: Part 3d
		// TODO: Part 2e
		//mProxy.RotateXLocalF(matrixBuffer.view, G_DEGREE_TO_RADIAN(-18), matrixBuffer.view);
		//mProxy.InverseF(view, view);
		mProxy.LookAtLHF(eye, at, up, view);

		mProxy.MultiplyMatrixF(view, perspective, matrixBuffer.view);

		for (size_t i = 0; i < 5; i++)
		{
			walls[i].view = matrixBuffer.view;
		}

		/***************** GEOMETRY INTIALIZATION ******************/
		// Grab the device & physical device so we can allocate some stuff
		VkPhysicalDevice physicalDevice = nullptr;
		vlk.GetDevice((void**)&device);
		vlk.GetPhysicalDevice((void**)&physicalDevice);

		// TODO: Part 1b
		float triOutline[] = {
			0.0f, 0.5f,
			0.5f, -0.5f,
			0.5f, -0.5f,
			-0.5f, -0.5f,
			-0.5f, -0.5f,
			0.0f, 0.5f
		};
		// TODO: Part 1c
		Vertex triOutVert[] = {
			{0.0f, 0.5f, 0, 1},
			{0.5f, -0.5f, 0, 1},
			{0.5f, -0.5f, 0, 1},
			{-0.5f, -0.5f, 0, 1},
			{-0.5f, -0.5f, 0, 1},
			{0.0f, 0.5f, 0, 1},
		};
		// Create Vertex Buffer
		float verts[] = {
			   0,   0.5f,
			 0.5f, -0.5f,
			-0.5f, -0.5f
		};
		// TODO: Part 1d
		Vertex grid[104] = {

		};

		float y = -0.5f;
		float x = -0.5f;
		for (int i = 0; i < 52; i += 2)
		{
			/*Horizontal*/
			grid[i] = { 0.5f, y, 0, 1 };
			grid[i + 1] = { -0.5f, y, 0, 1 };
			y += 0.04;
		}

		for (int i = 52; i < 104; i += 2)
		{
			/*Vertical*/
			grid[i] = { x, 0.5f, 0, 1 };
			grid[i + 1] = { x, -0.5f, 0, 1 };
			x += 0.04;
		}

		// Transfer triangle data to the vertex buffer. (staging would be prefered here)
		GvkHelper::create_buffer(physicalDevice, device, sizeof(grid),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vertexHandle, &vertexData);
		GvkHelper::write_to_buffer(device, vertexData, grid, sizeof(grid));



		/***************** SHADER INTIALIZATION ******************/
		// Intialize runtime shader compiler HLSL -> SPIRV
		shaderc_compiler_t compiler = shaderc_compiler_initialize();
		shaderc_compile_options_t options = shaderc_compile_options_initialize();
		shaderc_compile_options_set_source_language(options, shaderc_source_language_hlsl);
		// TODO: Part 3C
		shaderc_compile_options_set_invert_y(options, false);
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
		assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		assembly_create_info.primitiveRestartEnable = false;
		// Vertex Input State
		// TODO: Part 1c
		VkVertexInputBindingDescription vertex_binding_description = {};
		vertex_binding_description.binding = 0;
		vertex_binding_description.stride = sizeof(float) * 9;
		vertex_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		// TODO: Part 1c
		VkVertexInputAttributeDescription vertex_attribute_description[] = {
			{ 0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0 } //uv, normal, etc....
			//{ 1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 2} //uv, normal, etc....
		};
		VkPipelineVertexInputStateCreateInfo input_vertex_info = {};
		input_vertex_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		input_vertex_info.vertexBindingDescriptionCount = 1;
		input_vertex_info.pVertexBindingDescriptions = &vertex_binding_description;
		input_vertex_info.vertexAttributeDescriptionCount = ARRAYSIZE(vertex_attribute_description);
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
		// TODO: Part 2c
		VkPushConstantRange pushConstantRange = {};
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(WM);
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		// Descriptor pipeline layout
		VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
		pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_create_info.setLayoutCount = 0;
		pipeline_layout_create_info.pSetLayouts = VK_NULL_HANDLE;
		pipeline_layout_create_info.pushConstantRangeCount = 1; // TODO: Part 2d 
		pipeline_layout_create_info.pPushConstantRanges = &pushConstantRange; // TODO: Part 2d
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
		// TODO: Part 3a
		// TODO: Part 3b
		// TODO: Part 2b

			// TODO: Part 2f, Part 3b
		// TODO: Part 2d
		vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0, sizeof(WM), &matrixBuffer);

		////// now we can draw
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexHandle, offsets);
		vkCmdDraw(commandBuffer, 104, 1, 0, 0); // TODO: Part 1b // TODO: Part 1c

		for (size_t i = 0; i < 5; i++)
		{
			vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0, sizeof(WM), &walls[i]);

			vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexHandle, offsets);
			vkCmdDraw(commandBuffer, 104, 1, 0, 0); // TODO: Part 1b // TODO: Part 1c 
		}

		// TODO: Part 3e
	}
	// TODO: Part 4b 
	void UpdateCamera()
	{
		GW::MATH::GMATRIXF cam = GW::MATH::GIdentityMatrixF;

		auto now = std::chrono::steady_clock::now();
		deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(now - lastUpdate).count() / 1000000.0f; //seconds
		lastUpdate = now;

		// TODO: Part 4c
		mProxy.InverseF(view, cam);
		// TODO: Part 4d
		float y = 0.0f;

		float totalY = 0.0f;
		float totalZ = 0.0f;
		float totalX = 0.0f;

		const float Camera_Speed = 0.3f;
		float spaceKeyState = 0.0f;
		float leftShiftState = 0.0f;
		float rightTriggerState = 0.0f;
		float leftTriggerState = 0.0f;
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
		gInputProxy.GetMouseDelta(mouseDeltaX, mouseDeltaY);
		float rightStickYaxis = 0.0f;
		gControllerProxy.GetState(0, G_RY_AXIS, rightStickYaxis); 
		float rightStickXaxis = 0.0f;
		gControllerProxy.GetState(0, G_RX_AXIS, rightStickXaxis);

		float perFrameSpeed = 0.0f;

		if (G_PASS(gInputProxy.GetState(G_KEY_SPACE, spaceKeyState)) &&
			spaceKeyState != 0 ||

			G_PASS(gInputProxy.GetState(G_KEY_LEFTSHIFT, leftShiftState)) && 
			leftShiftState != 0 ||

			G_PASS(gControllerProxy.GetState(0,G_RIGHT_TRIGGER_AXIS, rightTriggerState)) && 
			rightTriggerState != 0 ||

			G_PASS(gControllerProxy.GetState(0,G_LEFT_TRIGGER_AXIS, leftTriggerState)) &&
			leftTriggerState != 0 )
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
		 float totalPitch = G_DEGREE_TO_RADIAN(65) * mouseDeltaY / screenHeight + rightStickYaxis * -thumbSpeed;
		 GW::MATH::GMATRIXF pitchMatrix = GW::MATH::GIdentityMatrixF;
		 mProxy.RotationYawPitchRollF(0, totalPitch, 0, pitchMatrix);
		 mProxy.MultiplyMatrixF(pitchMatrix, cam, cam);
		// TODO: Part 4g
		 float totalYaw = G_DEGREE_TO_RADIAN(65) * aspect * mouseDeltaX / screenWidth + rightStickXaxis * thumbSpeed;
		 GW::MATH::GMATRIXF yawMatrix = GW::MATH::GIdentityMatrixF;
		 mProxy.RotationYawPitchRollF(totalYaw, 0, 0, yawMatrix);
		 GW::MATH::GMATRIXF camSave = cam;
		 mProxy.MultiplyMatrixF(cam, yawMatrix, cam);
		 cam = camSave;
		// TODO: Part 4c
		 mProxy.InverseF(cam, view);
		 mProxy.MultiplyMatrixF(view, perspective, matrixBuffer.view);
		 for (size_t i = 0; i < 5; i++)
		 {
			 walls[i].view = matrixBuffer.view;
		 }
	}
private:
	void CleanUp()
	{
		// wait till everything has completed
		vkDeviceWaitIdle(device);
		// Release allocated buffers, shaders & pipeline
		vkDestroyBuffer(device, vertexHandle, nullptr);
		vkFreeMemory(device, vertexData, nullptr);
		vkDestroyShaderModule(device, vertexShader, nullptr);
		vkDestroyShaderModule(device, pixelShader, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyPipeline(device, pipeline, nullptr);
	}
};
