// This has been adapted from the Vulkan tutorial

#include "MyProject.hpp"

const std::string MODEL_PATH = "models/";
const std::string TEXTURE_PATH = "textures/";

// The uniform buffer object used in this example
// have 2 sets: set 0: view and proj and set 1: model matrix and texture
// set 0 biunding 0: view, proj
// set 1 binding 0: model matrix
// set 1 binding 1: texture
// in this way set 1 cange per object
struct globalUniformBufferObject
{
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
    alignas(16) float time;
    alignas(16) glm::vec3 eyePos;
    alignas(16) glm::vec4 coneInOutDecayExp;
    alignas(16) glm::vec3 cameraDir;
};

struct UniformBufferObject
{
	alignas(16) glm::mat4 model;
    alignas(16) int isFlowingColor;
    alignas(16) glm::vec3 highlightColor;
};

// MAIN !
class MyProject : public BaseProject
{
protected:
	// Here you list all the Vulkan objects you need:
    float lookYaw = 3.0;
    float lookPitch = 0.0;
    float lookRoll = 0.0;
    glm::vec3 RobotPos = glm::vec3(11.0f,1.0f,-25.0f);
    glm::vec3 highLightColors[5] = {glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, 0.5, 1.0), glm::vec3(0.4, 0.1, 1.0)};
    int colorSelectorToUnlock = 0;
    int colorSelFreezed = -1;
    bool doorUnlocked = false;

	// Descriptor Layouts [what will be passed to the shaders]
	// which variable will be passed in a shader of which type and defines the bindings
	DescriptorSetLayout DSLglobal;
	DescriptorSetLayout DSLobj;

	// Pipelines [Shader couples]
	Pipeline P1;

	// Models, textures and Descriptors (values assigned to the uniforms)
    // (SLOT BODY)
	Model M_SlBody;
	Texture T_SlBody;
    DescriptorSet DS_SlBody; // instances of DSLobj, are elments that are passed to shaders
    
    // for each model (HANDLE)
	Model M_SlHandle;
	Texture T_SlHandle;
	DescriptorSet DS_SlHandle; // instances of DSLobj
    DescriptorSet DS_SlHandle2; // instances of DSLobj
    // ---------
    
    Model M_Door;
    Texture T_Door;
    DescriptorSet DS_Door; // instances of DSLobj
    
    Model M_IntBlock;
    Texture T_IntBlock;
    DescriptorSet DS_IntBlock; // instances of DSLobj
    
    Model M_Hint;
    Texture T_Hint;
    DescriptorSet DS_Hint; // instances of DSLobj
    
    
    
    DescriptorSet DS_global;

	// Here you set the main application parameters
	void setWindowParameters()
	{
		// window size, titile and initial background
		windowWidth = 1024;
		windowHeight = 768;
		windowTitle = "My Project";
		initialBackgroundColor = {0.0f, 0.0f, 0.0f, 1.0f};

		// Descriptor pool sizes
		uniformBlocksInPool = 7; // how many descriptor set you're going to use
		texturesInPool = 6;
		setsInPool = 7; //handle, body, global for now + 3 wheels
	}

	// Here you load and setup all your Vulkan objects
	void localInit()
	{
		// Descriptor Layouts [what will be passed to the shaders]
		DSLobj.init(this, {// this array contains the binding:
							  // first  element : the binding number
							  // second element : the time of element (buffer or texture)
							  // third  element : the pipeline stage where it will be used
							  {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS},
							  {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}});

		// Descriptor Layouts [what will be passed to the shaders]
        // the gblobal does not have textures, this is why we deleted the second element
		DSLglobal.init(this, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}});

		// Pipelines [Shader couples]
		// The last array, is a vector of pointer to the layouts of the sets that will
		// be used in this pipeline. The first element will be set 0, and so on..
		P1.init(this, "shaders/vert.spv", "shaders/frag.spv", {&DSLglobal, &DSLobj}); //the first changes less freq while the last more frequently.

		// Models, textures and Descriptors (values assigned to the uniforms)
		M_SlBody.init(this, MODEL_PATH + "newcave.obj");
		T_SlBody.init(this, TEXTURE_PATH + "block.png");
		DS_SlBody.init(this, &DSLobj, {// the second parameter, is a pointer to the Uniform Set Layout of this set
										  // the last parameter is an array, with one element per binding of the set.
										  // first  elmenet : the binding number
										  // second element : UNIFORM or TEXTURE (an enum) depending on the type
										  // third  element : only for UNIFORMs, the size of the corresponding C++ object
										  // fourth element : only for TEXTUREs, the pointer to the corresponding texture object
										  {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
										  {1, TEXTURE, 0, &T_SlBody}});
        // (HANDLE) for each model
		M_SlHandle.init(this, MODEL_PATH + "block.obj");
		T_SlHandle.init(this, TEXTURE_PATH + "redBrick.png");
		DS_SlHandle.init(this, &DSLobj, {// it uses same layout but we set a different instance of it
											{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
											{1, TEXTURE, 0, &T_SlHandle}});
        DS_SlHandle2.init(this, &DSLobj, {// it uses same layout but we set a different instance of it
                                            {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                                            {1, TEXTURE, 0, &T_SlHandle}});
        
        // ---------------
        
        M_IntBlock.init(this, MODEL_PATH + "block.obj");
        T_IntBlock.init(this, TEXTURE_PATH + "block.png");
        DS_IntBlock.init(this, &DSLobj, {// it uses same layout but we set a different instance of it
                                            {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                                            {1, TEXTURE, 0, &T_IntBlock}});
        
        M_Door.init(this, MODEL_PATH + "door.obj");
        T_Door.init(this, TEXTURE_PATH + "block.png");
        DS_Door.init(this, &DSLobj, {// it uses same layout but we set a different instance of it
                                            {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                                            {1, TEXTURE, 0, &T_Door}});
        
        M_Hint.init(this, MODEL_PATH + "hint.obj");
        T_Hint.init(this, TEXTURE_PATH + "hint.png");
        DS_Hint.init(this, &DSLobj, {// it uses same layout but we set a different instance of it
                                            {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                                            {1, TEXTURE, 0, &T_Hint}});
        
        
        // add a new init for the global DS
        DS_global.init(this, &DSLglobal, {{0, UNIFORM, sizeof(globalUniformBufferObject), nullptr}});
        // ---------------
	}

	// Here you destroy all the objects you created!
	void localCleanup()
	{
		// model 1 cleanup
		DS_SlBody.cleanup();
		T_SlBody.cleanup();
		M_SlBody.cleanup();
		// model 2 cleanup
		DS_SlHandle.cleanup();
        DS_SlHandle2.cleanup();
		T_SlHandle.cleanup();
		M_SlHandle.cleanup();
        // interactive block cleanup
        DS_IntBlock.cleanup();
        T_IntBlock.cleanup();
        M_IntBlock.cleanup();
        // door cleanup
        DS_Door.cleanup();
        T_Door.cleanup();
        M_Door.cleanup();
        // hint cleanup
        DS_Hint.cleanup();
        T_Hint.cleanup();
        M_Hint.cleanup();
        
        DS_global.cleanup();

		P1.cleanup();
		DSLglobal.cleanup();
        DSLobj.cleanup();
	}

	// Here it is the creation of the command buffer:
	// You send to the GPU all the objects you want to draw,
	// with their buffers and textures
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage)
	{

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
						  P1.graphicsPipeline);
        // GLOBAL DS
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 0, 1, &DS_global.descriptorSets[currentImage], //the first integer parameter is the set, global has set=0 objects will have set=1 then
                                0, nullptr);

		// MODEL OF BODY
		VkBuffer vertexBuffers[] = {M_SlBody.vertexBuffer};
		// property .vertexBuffer of models, contains the VkBuffer handle to its vertex buffer
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		// property .indexBuffer of models, contains the VkBuffer handle to its index buffer
		vkCmdBindIndexBuffer(commandBuffer, M_SlBody.indexBuffer, 0,
							 VK_INDEX_TYPE_UINT32);

		// property .pipelineLayout of a pipeline contains its layout.
		// property .descriptorSets of a descriptor set contains its elements.
		vkCmdBindDescriptorSets(commandBuffer,
								VK_PIPELINE_BIND_POINT_GRAPHICS,
								P1.pipelineLayout, 1, 1, &DS_SlBody.descriptorSets[currentImage],
								0, nullptr);

		// property .indices.size() of models, contains the number of triangles * 3 of the mesh.
		vkCmdDrawIndexed(commandBuffer,
						 static_cast<uint32_t>(M_SlBody.indices.size()), 1, 0, 0, 0);
        //----------------

		// MODEL OF Handle
		VkBuffer vertexBuffersHandle[] = {M_SlHandle.vertexBuffer};
		VkDeviceSize offsetsHandle[] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersHandle, offsetsHandle);
		vkCmdBindIndexBuffer(commandBuffer, M_SlHandle.indexBuffer, 0,
							 VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffer,
								VK_PIPELINE_BIND_POINT_GRAPHICS,
								P1.pipelineLayout, 1, 1, &DS_SlHandle.descriptorSets[currentImage], //particular objects DS (descriptors) will have set=1 (it's the first integer parameter)
								0, nullptr);
		vkCmdDrawIndexed(commandBuffer,
						 static_cast<uint32_t>(M_SlHandle.indices.size()), 1, 0, 0, 0);
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 1, 1, &DS_SlHandle2.descriptorSets[currentImage], //particular objects DS (descriptors) will have set=1 (it's the first integer parameter)
                                0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M_SlHandle.indices.size()), 1, 0, 0, 0);
        //----------------
        
        // MODEL OF Interactive Block
        VkBuffer vertexBuffersIntBlock[] = {M_IntBlock.vertexBuffer};
        VkDeviceSize offsetsIntBlock[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersIntBlock, offsetsIntBlock);
        vkCmdBindIndexBuffer(commandBuffer, M_IntBlock.indexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 1, 1, &DS_IntBlock.descriptorSets[currentImage], //particular objects DS (descriptors) will have set=1 (it's the first integer parameter)
                                0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M_IntBlock.indices.size()), 1, 0, 0, 0);
        //----------------
        
        // MODEL OF Door
        VkBuffer vertexBuffersDoor[] = {M_Door.vertexBuffer};
        VkDeviceSize offsetsDoor[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersDoor, offsetsDoor);
        vkCmdBindIndexBuffer(commandBuffer, M_Door.indexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 1, 1, &DS_Door.descriptorSets[currentImage], //particular objects DS (descriptors) will have set=1 (it's the first integer parameter)
                                0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M_Door.indices.size()), 1, 0, 0, 0);
        //----------------
        
        // MODEL OF Hint
        VkBuffer vertexBuffersHint[] = {M_Hint.vertexBuffer};
        VkDeviceSize offsetsHint[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersHint, offsetsHint);
        vkCmdBindIndexBuffer(commandBuffer, M_Hint.indexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 1, 1, &DS_Hint.descriptorSets[currentImage], //particular objects DS (descriptors) will have set=1 (it's the first integer parameter)
                                0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M_Hint.indices.size()), 1, 0, 0, 0);
        //----------------
	}

	// Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();
        static float lastTime = 0.0f;
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        float deltaT = time - lastTime;
        lastTime = time;
        const float ROT_SPEED = glm::radians(90.0f);
        const float MOVE_SPEED = 6.75f;
        
        static double old_xpos = 0, old_ypos = 0;
        double xpos, ypos;
        //double m_dx = xpos - old_xpos;
        //double m_dy = ypos - old_ypos;
        old_xpos = xpos; old_ypos = ypos;
        
        Point polygonFirstLevel[]={
            {1.977f, -0.12f},{-8.0f, -0.29f},{-8.1f, 4.9f},{-17.977f, 4.95f},{-18.40947f, 15.119062f},{-13.2749f, 15.286366f}, {-13.08979f, -0.321581},{-2.94981f, 32.661884f},{-2.756874f, 30.245684},{7.059189f, 30.348223f},{7.351864f, 27.670744f},{17.165037f, 27.727234f},{17.309513f, 25.223669f},{27.082043f, 25.309875f},{27.167667f, 15.291109f},{32.010273f, 15.222846f},{32.212494f, 5.442165f},{37.041645f, 5.490769f},{37.258324f, -4.548279f},{42.106056f, -4.725879f},{42.303799f, -24.741739f},{51.765488f, -24.710552f},{51.789332f, 4.637737f},{46.881699f, 4.821360f},{46.808151f, 14.725124f},{41.870975f, 14.739830f},{41.740196f, 24.785589f},{36.900578f, 24.698938f},{36.700233f, 34.750481f},{16.906641f, 34.621841f},{16.750671f, 39.685867f},{6.838522f, 39.544998f},{6.710043f, 44.605782f},{-2.701247f, 44.647972f},{-3.126298f, 42.389423f},{-13.196265f, 42.362267f},{-13.263083f, 44.729404f},{-22.239576f, 44.574398f},{-22.488251f, 39.968781f},{-32.621956f,39.646076f},{-32.610435f, 34.810993f},{-37.548759f, 34.689541f},{-37.506943,24.909031f},{-47.633053f, 24.743914f},{-47.550144f, 20.606758f},{-42.709469f, 20.590010f},{-42.595543f, 12.155780f},{-37.737270f, 12.345304f},{-37.563602f, -5.191618f},{-47.347424f, -5.375168f},{-47.347515f, -14.043010f},{-19.012642f, -14.177714f},{-18.594564f, -9.578016f},{2.382220f, -9.686684f},{2.396673f, -19.569105f},{7.026602f, -19.527569f},{7.362133f, -30.314966f},{-3.171566f, -30.144741f},{-3.237250, -27.667923f},{-7.742736f, -27.791506f},{-7.834392f, -30.270039f},{-17.734526f, -30.351521f},{-17.885273f, -39.995483f},{-28.120893f, -40.382954f},{-28.367798f, -30.411938f},{-37.807617f, -30.382368f},{-37.922546f, -40.062504f},{-57.161739f, -40.423023f},{-57.090649f, -49.525463f},{-42.896645f, -49.734165f},{-42.822685f, -54.563705f},{-33.178314f, -54.498180f},{-33.264153f, -49.681889f},{-22.716454f, -49.681530f},{-22.596651f, -59.278316f},{-18.229576f, -59.338959f},{-18.274920f, -49.866566f},{-8.358553f, -49.806484f},{-8.410365f, -39.771515f},{2.154401f, -39.859848f},{2.274107f, -44.014206f},{11.837919f, -43.865822f},{11.794569f, -39.835190f},{16.274645f, -39.648861f},{16.734129f, -19.699081f},{21.426973f, -19.577007f},{21.458563f, -15.080132f},{11.877460f, -15.121017f},{11.906350f, -5.096962f},{6.868548f, -5.122917f},{6.713150f, 4.113907f},{2.215148f, 4.033896f}
        };
        
        glm::vec3 oldRobotPos = RobotPos;
        if(glfwGetKey(window, GLFW_KEY_LEFT)) {
            lookYaw += deltaT * ROT_SPEED;
        }
        if(glfwGetKey(window, GLFW_KEY_RIGHT)) {
            lookYaw -= deltaT * ROT_SPEED;
        }
        if(glfwGetKey(window, GLFW_KEY_UP)) {
            lookPitch += deltaT * ROT_SPEED;
        }
        if(glfwGetKey(window, GLFW_KEY_DOWN)) {
            lookPitch -= deltaT * ROT_SPEED;
        }
        if(glfwGetKey(window, GLFW_KEY_Q)) {
            lookRoll -= deltaT * ROT_SPEED;
        }
        if(glfwGetKey(window, GLFW_KEY_E)) {
            lookRoll += deltaT * ROT_SPEED;
        }
        if(glfwGetKey(window, GLFW_KEY_A)) {
            RobotPos -= MOVE_SPEED * glm::vec3(glm::rotate(glm::mat4(1.0f), lookYaw,
                                    glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(1,0,0,1)) * deltaT;
        }
        if(glfwGetKey(window, GLFW_KEY_D)) {
            RobotPos += MOVE_SPEED * glm::vec3(glm::rotate(glm::mat4(1.0f), lookYaw,
                                    glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(1,0,0,1)) * deltaT;
        }
        if(glfwGetKey(window, GLFW_KEY_W)) {
            RobotPos -= MOVE_SPEED * glm::vec3(glm::rotate(glm::mat4(1.0f), lookYaw,
                                    glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(0,0,1,1)) * deltaT;
        }
        if(glfwGetKey(window, GLFW_KEY_S)) {
            RobotPos += MOVE_SPEED * glm::vec3(glm::rotate(glm::mat4(1.0f), lookYaw,
                                    glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(0,0,1,1)) * deltaT;
        }
        if(glfwGetKey(window, GLFW_KEY_F)) {
            RobotPos += MOVE_SPEED * glm::vec3(0.0f, deltaT, 0.0f);
        }
        if(glfwGetKey(window, GLFW_KEY_G)) {
            RobotPos -= MOVE_SPEED * glm::vec3(0.0f, deltaT, 0.0f);
        }
        if (!isInside(polygonFirstLevel, sizeof(polygonFirstLevel)/sizeof(polygonFirstLevel[0]), {RobotPos[0], RobotPos[2]})) {
            RobotPos = oldRobotPos;
        }
        std::cout << std::to_string(RobotPos[0]) << " " << std::to_string(RobotPos[1])<< " " << std::to_string(RobotPos[2])<< " " << "\n";
        
        
        
        int nearest_plat_index = getNearestPlatform(RobotPos);
        // possible code to move an object after an interaction (going up and down like a platform)
        float animationDuration = 2; // seconds of animation
        static int isMoving = 0;
        static auto interactionStartTime = std::chrono::high_resolution_clock::now();
        static glm::vec3 handlePosStart = glm::vec3(0.0f, -1.90f, 0.1f);
        static glm::vec3 handlePosEnd = glm::vec3(0.0f, 8.0f, 0.1f);
        static glm::vec3 handlePos = handlePosStart;
        if (!isMoving && glfwGetKey(window, GLFW_KEY_SPACE)) {
            std::cout << "nearest plat: " << nearest_plat_index;
            isMoving = 1;
            interactionStartTime = std::chrono::high_resolution_clock::now();
        }
        if (isMoving){
            bool cameraNeedToMove = isCameraOnPlatform(getVerticesOfPlatform(nearest_plat_index), RobotPos);
            std::cout << "camera need to move: " << cameraNeedToMove;
            float deltaTimeAnimation = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - interactionStartTime).count();
            if (deltaTimeAnimation >= animationDuration) {
                
                handlePos = handlePosEnd;
                isMoving = 0;
                glm::vec3 handlePosTemp = handlePosEnd;
                handlePosEnd = handlePosStart;
                handlePosStart = handlePosTemp;
            } else {
                handlePos = glm::vec3(handlePosStart[0] + ((handlePosEnd[0]-handlePosStart[0])/animationDuration)*deltaTimeAnimation,
                                      handlePosStart[1] + ((handlePosEnd[1]-handlePosStart[1])/animationDuration)*deltaTimeAnimation,
                                      handlePosStart[2] + ((handlePosEnd[2]-handlePosStart[2])/animationDuration)*deltaTimeAnimation);
                if (cameraNeedToMove) {
                    float height_diff = handlePosEnd[1] - handlePosStart[1];
                    RobotPos[1] += (height_diff/animationDuration)*deltaT;
                }
            }
        }
        // ------------------- animation code
        
        // Animation of door
        // possible code to move an object after an interaction (DOOR)
        int selColor = (int)std::floor(time) % 5;
        static int doorIsMoving = 0;
        static int blockColorFlowing = 1;
        static glm::vec3 doorPosStart = glm::vec3(0.0f, 0.0f, 0.0f);
        static glm::vec3 doorPosEnd = glm::vec3(0.0f, -8.0f, 0.0f);
        static glm::vec3 doorPos = doorPosStart;
        bool isOnIntBlock = isCameraOnPlatform(getVerticesOfIntBlock(), RobotPos);
        bool isOnRightColor = colorSelectorToUnlock == colorSelFreezed;
        if (!doorIsMoving && isOnIntBlock && isOnRightColor && !doorUnlocked) {
            doorIsMoving = 1;
            doorUnlocked = true;
            interactionStartTime = std::chrono::high_resolution_clock::now();
        }
        if (isOnIntBlock && blockColorFlowing) {
            colorSelFreezed = selColor;
            blockColorFlowing = 0;
        }
        if (!isOnIntBlock && !doorUnlocked){
            blockColorFlowing = 1;
        }
        if (doorIsMoving){
            float deltaTimeAnimation = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - interactionStartTime).count();
            if (deltaTimeAnimation >= animationDuration) {
                doorPos = doorPosEnd;
                doorIsMoving = 0;
                glm::vec3 doorPosTemp = doorPosEnd;
                doorPosEnd = doorPosStart;
                doorPosStart = doorPosTemp;
            } else {
                doorPos = glm::vec3(doorPosStart[0] + ((doorPosEnd[0]-doorPosStart[0])/animationDuration)*deltaTimeAnimation,
                                      doorPosStart[1] + ((doorPosEnd[1]-doorPosStart[1])/animationDuration)*deltaTimeAnimation,
                                      doorPosStart[2] + ((doorPosEnd[2]-doorPosStart[2])/animationDuration)*deltaTimeAnimation);
            }
        }
        // ------------------- animation code

        
        

        void *data;
        
		globalUniformBufferObject gubo{};
        UniformBufferObject ubo{};
        gubo.view = LookInDirMat(RobotPos, glm::vec3(lookYaw, lookPitch, lookRoll));
		gubo.proj = glm::perspective(glm::radians(45.0f),
									swapChainExtent.width / (float)swapChainExtent.height,
									0.1f, 150.0f);
		gubo.proj[1][1] *= -1;
        gubo.time = glm::fract(time);
        gubo.eyePos = RobotPos;
        gubo.coneInOutDecayExp = glm::vec4(0.0f, 0.3f, 2.0f, 2.0f);
        float direction_x = cos((lookYaw)) * cos((lookPitch));
        float direction_y = sin((lookPitch));
        float direction_z = sin((lookYaw)) * cos((lookPitch));
        gubo.cameraDir = glm::normalize(glm::vec3(direction_x, direction_y, direction_z));
        //std::cout << gubo.cameraDir[0] << " " << gubo.cameraDir[1] << " " << gubo.cameraDir[2] << "\n";
        
        
        // Global
        vkMapMemory(device, DS_global.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(gubo), 0, &data);
        memcpy(data, &gubo, sizeof(gubo));
        vkUnmapMemory(device, DS_global.uniformBuffersMemory[0][currentImage]);

        ubo.isFlowingColor = 0;
		// doing for every model or better for every (DS_) -- HERE: SLBody --
		// Here is where you actually update your uniforms
		// uniformBuffersMemory[0] -> the 0 is the binding of the uniform you're going to change
		ubo.model = glm::mat4(1.0f);
		vkMapMemory(device, DS_SlBody.uniformBuffersMemory[0][currentImage], 0,
					sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, DS_SlBody.uniformBuffersMemory[0][currentImage]);
		// ------------

		// (HANDLE) doing for every model or better for every (DS_)
		ubo.model = glm::translate(glm::mat4(1), handlePos); // you can modify your ubo for each DS before passing it
		vkMapMemory(device, DS_SlHandle.uniformBuffersMemory[0][currentImage], 0,
					sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, DS_SlHandle.uniformBuffersMemory[0][currentImage]);
		// ------------
        // (HANDLE2) doing for every model or better for every (DS_)
        ubo.model = glm::translate(glm::mat4(1), glm::vec3(-17.9, handlePos[1], 12.0)); // you can modify your ubo for each DS before passing it
        vkMapMemory(device, DS_SlHandle2.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_SlHandle2.uniformBuffersMemory[0][currentImage]);
        // ------------
        
        // (INTBLOCK) doing for every model or better for every (DS_)
        ubo.model = glm::translate(glm::mat4(1), glm::vec3(-26.0, -1.8, 33.0)) *
                    glm::scale(glm::mat4(1), glm::vec3(0.5, 0.5, 0.5)); // you can modify your ubo for each DS before passing it
        ubo.isFlowingColor = 0;
        ubo.highlightColor = highLightColors[selColor];
        if (doorUnlocked || !blockColorFlowing) {
            ubo.highlightColor = highLightColors[colorSelFreezed];
        }
        vkMapMemory(device, DS_IntBlock.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_IntBlock.uniformBuffersMemory[0][currentImage]);
        // ------------
        
        // (DOOR) doing for every model or better for every (DS_)
        ubo.isFlowingColor = 0;
        ubo.model = glm::translate(glm::mat4(1), doorPos); // you can modify your ubo for each DS before passing it
        vkMapMemory(device, DS_Door.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_Door.uniformBuffersMemory[0][currentImage]);
        ubo.highlightColor = glm::vec3(0.0, 0.0, 0.0); //set back to null highlight
        // ------------
        
        // (HINT) doing for every model or better for every (DS_)
        ubo.model = glm::mat4(1.0);
        vkMapMemory(device, DS_Hint.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_Hint.uniformBuffersMemory[0][currentImage]);
        // ------------
        
	}
};

// This is the main: probably you do not need to touch this!
int main()
{
	MyProject app;

	try
	{
		app.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
