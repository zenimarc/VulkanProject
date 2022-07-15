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
        //std::cout << std::to_string(RobotPos[0]) << " " << std::to_string(RobotPos[1])<< " " << std::to_string(RobotPos[2])<< " " << "\n";
        
        
        
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
