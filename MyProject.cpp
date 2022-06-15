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
};

struct UniformBufferObject
{
	alignas(16) glm::mat4 model;
};

// MAIN !
class MyProject : public BaseProject
{
protected:
	// Here you list all the Vulkan objects you need:

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
    // ---------
    
    // for each model (WHEELS)
    Model M_SlWheel;
    Texture T_SlWheel;
    DescriptorSet DS_SlWheel1; // instances three times because same model with same texture but diff pos.
    DescriptorSet DS_SlWheel2;
    DescriptorSet DS_SlWheel3;
    // ---------
    
    
    DescriptorSet DS_global;

	// Here you set the main application parameters
	void setWindowParameters()
	{
		// window size, titile and initial background
		windowWidth = 800;
		windowHeight = 600;
		windowTitle = "My Project";
		initialBackgroundColor = {0.0f, 0.0f, 0.0f, 1.0f};

		// Descriptor pool sizes
		uniformBlocksInPool = 6; // how many descriptor set you're going to use
		texturesInPool = 5;
		setsInPool = 6; //handle, body, global for now + 3 wheels
	}

	// Here you load and setup all your Vulkan objects
	void localInit()
	{
		// Descriptor Layouts [what will be passed to the shaders]
		DSLobj.init(this, {// this array contains the binding:
							  // first  element : the binding number
							  // second element : the time of element (buffer or texture)
							  // third  element : the pipeline stage where it will be used
							  {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
							  {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}});

		// Descriptor Layouts [what will be passed to the shaders]
        // the gblobal does not have textures, this is why we deleted the second element
		DSLglobal.init(this, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}});

		// Pipelines [Shader couples]
		// The last array, is a vector of pointer to the layouts of the sets that will
		// be used in this pipeline. The first element will be set 0, and so on..
		P1.init(this, "shaders/vert.spv", "shaders/frag.spv", {&DSLglobal, &DSLobj}); //the first changes less freq while the last more frequently.

		// Models, textures and Descriptors (values assigned to the uniforms)
		M_SlBody.init(this, MODEL_PATH + "SlotBody.obj");
		T_SlBody.init(this, TEXTURE_PATH + "SlotBody.png");
		DS_SlBody.init(this, &DSLobj, {// the second parameter, is a pointer to the Uniform Set Layout of this set
										  // the last parameter is an array, with one element per binding of the set.
										  // first  elmenet : the binding number
										  // second element : UNIFORM or TEXTURE (an enum) depending on the type
										  // third  element : only for UNIFORMs, the size of the corresponding C++ object
										  // fourth element : only for TEXTUREs, the pointer to the corresponding texture object
										  {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
										  {1, TEXTURE, 0, &T_SlBody}});
        // (HANDLE) for each model
		M_SlHandle.init(this, MODEL_PATH + "SlotHandle.obj");
		T_SlHandle.init(this, TEXTURE_PATH + "SlotHandle.png");
		DS_SlHandle.init(this, &DSLobj, {// it uses same layout but we set a different instance of it
											{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
											{1, TEXTURE, 0, &T_SlHandle}});
        
        // ---------------
        
        // (WHEELS) for each model
        M_SlWheel.init(this, MODEL_PATH + "SlotWheel.obj");
        T_SlWheel.init(this, TEXTURE_PATH + "SlotWheel.png");
        DS_SlWheel1.init(this, &DSLobj, {
                                            {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                                            {1, TEXTURE, 0, &T_SlWheel}});
        DS_SlWheel2.init(this, &DSLobj, {
                                            {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                                            {1, TEXTURE, 0, &T_SlWheel}});
        DS_SlWheel3.init(this, &DSLobj, {
                                            {0, UNIFORM, sizeof(UniformBufferObject), nullptr},
                                            {1, TEXTURE, 0, &T_SlWheel}});
        
        
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
		T_SlHandle.cleanup();
		M_SlHandle.cleanup();
        // 3 wheels cleanup
        DS_SlWheel1.cleanup();
        DS_SlWheel2.cleanup();
        DS_SlWheel3.cleanup();
        T_SlWheel.cleanup();
        M_SlWheel.cleanup();
        
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
		VkBuffer vertexBuffers[] = {M_SlBody.vertexBuffer}; // possibly add more vertexBuffers
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
        //----------------
        
        // MODEL OF Wheels
        VkBuffer vertexBuffersWheels[] = {M_SlWheel.vertexBuffer};
        VkDeviceSize offsetsWheels[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersWheels, offsetsWheels);
        vkCmdBindIndexBuffer(commandBuffer, M_SlWheel.indexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 1, 1, &DS_SlWheel1.descriptorSets[currentImage], //particular objects DS (descriptors) will have set=1 (it's the first integer parameter)
                                0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M_SlWheel.indices.size()), 1, 0, 0, 0);
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 1, 1, &DS_SlWheel2.descriptorSets[currentImage], //since 3 wheels (3 DS) we replicate this bind
                                0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M_SlWheel.indices.size()), 1, 0, 0, 0);
        vkCmdBindDescriptorSets(commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                P1.pipelineLayout, 1, 1, &DS_SlWheel3.descriptorSets[currentImage], //particular objects DS (descriptors) will have set=1 (it's the first integer parameter)
                                0, nullptr);
        vkCmdDrawIndexed(commandBuffer,
                         static_cast<uint32_t>(M_SlWheel.indices.size()), 1, 0, 0, 0);
        //----------------
	}

	// Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        void *data;
        
		globalUniformBufferObject gubo{};
        UniformBufferObject ubo{};
		gubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
							   glm::vec3(0.0f, 0.0f, 0.0f),
							   glm::vec3(0.0f, 1.0f, 0.0f));
		gubo.proj = glm::perspective(glm::radians(45.0f),
									swapChainExtent.width / (float)swapChainExtent.height,
									0.1f, 10.0f);
		gubo.proj[1][1] *= -1;
        
        // Global
        vkMapMemory(device, DS_global.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(gubo), 0, &data);
        memcpy(data, &gubo, sizeof(gubo));
        vkUnmapMemory(device, DS_global.uniformBuffersMemory[0][currentImage]);

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
		ubo.model = glm::translate(glm::mat4(1), glm::vec3(0.3f, 0.5f, -0.15f)); // you can modify your ubo for each DS before passing it
		vkMapMemory(device, DS_SlHandle.uniformBuffersMemory[0][currentImage], 0,
					sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, DS_SlHandle.uniformBuffersMemory[0][currentImage]);
		// ------------
        
        // (WHEEL1) doing for every model or better for every (DS_)
        ubo.model = glm::translate(glm::mat4(1), glm::vec3(-0.15f, 0.93f, -0.15f))
        * glm::rotate(glm::mat4(1), time * glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // you can modify your ubo for each DS before passing it
        vkMapMemory(device, DS_SlWheel1.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_SlWheel1.uniformBuffersMemory[0][currentImage]);
        // ------------
        // (WHEEL2) doing for every model or better for every (DS_)
        ubo.model = glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.93f, -0.15f)); // you can modify your ubo for each DS before passing it
        vkMapMemory(device, DS_SlWheel2.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_SlWheel2.uniformBuffersMemory[0][currentImage]);
        // ------------
        // (WHEEL3) doing for every model or better for every (DS_)
        ubo.model = glm::translate(glm::mat4(1), glm::vec3(0.15f, 0.93f, -0.15f)); // you can modify your ubo for each DS before passing it
        vkMapMemory(device, DS_SlWheel3.uniformBuffersMemory[0][currentImage], 0,
                    sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(device, DS_SlWheel3.uniformBuffersMemory[0][currentImage]);
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
