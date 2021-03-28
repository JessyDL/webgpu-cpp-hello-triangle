#include <memory>
#include <optional>
#include <string>

#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/html5.h>
#include <emscripten/html5_webgpu.h>
#include <webgpu/webgpu_cpp.h>

static constexpr auto DEFAULT_SWAPCHAIN_FORMAT = wgpu::TextureFormat::BGRA8Unorm;
static constexpr auto DEFAULT_DEPTH_FORMAT     = wgpu::TextureFormat::Depth32Float;


#define TODO(msg)                                                                 \
	{                                                                             \
		std::string endlined_msg { "todo: " };                                    \
		endlined_msg += msg;                                                      \
		endlined_msg += " (in %s:%i -> %s)\n";                                    \
		printf(endlined_msg.data(), __FILE__, __LINE__, __PRETTY_FUNCTION__);     \
	}
#define TODO_ONCE(msg)                                                            \
	{                                                                             \
		std::string endlined_msg { "todo: " };                                    \
		static bool has_run##__LINE__ {false};                                    \
		if(!(has_run##__LINE__))                                                  \
		{                                                                         \
			endlined_msg += msg;                                                  \
			endlined_msg += " (in %s:%i -> %s)\n";                                \
			printf(endlined_msg.data(), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
			has_run##__LINE__ = true;                                             \
		}                                                                         \
	}

constexpr uint32_t vert_spv[]
	{
		0x07230203,0x00010000,0x000d0008,0x00000018,
		0x00000000,0x00020011,0x00000001,0x0006000b,
		0x00000001,0x4c534c47,0x6474732e,0x3035342e,
		0x00000000,0x0003000e,0x00000000,0x00000001,
		0x0009000f,0x00000000,0x00000004,0x6e69616d,
		0x00000000,0x00000009,0x0000000b,0x00000012,
		0x00000015,0x00030003,0x00000002,0x000001c2,
		0x000a0004,0x475f4c47,0x4c474f4f,0x70635f45,
		0x74735f70,0x5f656c79,0x656e696c,0x7269645f,
		0x69746365,0x00006576,0x00080004,0x475f4c47,
		0x4c474f4f,0x6e695f45,0x64756c63,0x69645f65,
		0x74636572,0x00657669,0x00040005,0x00000004,
		0x6e69616d,0x00000000,0x00040005,0x00000009,
		0x6c6f6366,0x0000726f,0x00040005,0x0000000b,
		0x6c6f6376,0x0000726f,0x00060005,0x00000010,
		0x505f6c67,0x65567265,0x78657472,0x00000000,
		0x00060006,0x00000010,0x00000000,0x505f6c67,
		0x7469736f,0x006e6f69,0x00070006,0x00000010,
		0x00000001,0x505f6c67,0x746e696f,0x657a6953,
		0x00000000,0x00070006,0x00000010,0x00000002,
		0x435f6c67,0x4470696c,0x61747369,0x0065636e,
		0x00070006,0x00000010,0x00000003,0x435f6c67,
		0x446c6c75,0x61747369,0x0065636e,0x00030005,
		0x00000012,0x00000000,0x00030005,0x00000015,
		0x00736f70,0x00040047,0x00000009,0x0000001e,
		0x00000000,0x00040047,0x0000000b,0x0000001e,
		0x00000001,0x00050048,0x00000010,0x00000000,
		0x0000000b,0x00000000,0x00050048,0x00000010,
		0x00000001,0x0000000b,0x00000001,0x00050048,
		0x00000010,0x00000002,0x0000000b,0x00000003,
		0x00050048,0x00000010,0x00000003,0x0000000b,
		0x00000004,0x00030047,0x00000010,0x00000002,
		0x00040047,0x00000015,0x0000001e,0x00000000,
		0x00020013,0x00000002,0x00030021,0x00000003,
		0x00000002,0x00030016,0x00000006,0x00000020,
		0x00040017,0x00000007,0x00000006,0x00000004,
		0x00040020,0x00000008,0x00000003,0x00000007,
		0x0004003b,0x00000008,0x00000009,0x00000003,
		0x00040020,0x0000000a,0x00000001,0x00000007,
		0x0004003b,0x0000000a,0x0000000b,0x00000001,
		0x00040015,0x0000000d,0x00000020,0x00000000,
		0x0004002b,0x0000000d,0x0000000e,0x00000001,
		0x0004001c,0x0000000f,0x00000006,0x0000000e,
		0x0006001e,0x00000010,0x00000007,0x00000006,
		0x0000000f,0x0000000f,0x00040020,0x00000011,
		0x00000003,0x00000010,0x0004003b,0x00000011,
		0x00000012,0x00000003,0x00040015,0x00000013,
		0x00000020,0x00000001,0x0004002b,0x00000013,
		0x00000014,0x00000000,0x0004003b,0x0000000a,
		0x00000015,0x00000001,0x00050036,0x00000002,
		0x00000004,0x00000000,0x00000003,0x000200f8,
		0x00000005,0x0004003d,0x00000007,0x0000000c,
		0x0000000b,0x0003003e,0x00000009,0x0000000c,
		0x0004003d,0x00000007,0x00000016,0x00000015,
		0x00050041,0x00000008,0x00000017,0x00000012,
		0x00000014,0x0003003e,0x00000017,0x00000016,
		0x000100fd,0x00010038
	};

constexpr uint32_t frag_spv[]
	{
		0x07230203,0x00010000,0x000d0008,0x0000000d,
		0x00000000,0x00020011,0x00000001,0x0006000b,
		0x00000001,0x4c534c47,0x6474732e,0x3035342e,
		0x00000000,0x0003000e,0x00000000,0x00000001,
		0x0007000f,0x00000004,0x00000004,0x6e69616d,
		0x00000000,0x00000009,0x0000000b,0x00030010,
		0x00000004,0x00000007,0x00030003,0x00000002,
		0x000001c2,0x000a0004,0x475f4c47,0x4c474f4f,
		0x70635f45,0x74735f70,0x5f656c79,0x656e696c,
		0x7269645f,0x69746365,0x00006576,0x00080004,
		0x475f4c47,0x4c474f4f,0x6e695f45,0x64756c63,
		0x69645f65,0x74636572,0x00657669,0x00040005,
		0x00000004,0x6e69616d,0x00000000,0x00040005,
		0x00000009,0x6f6c6f63,0x00000072,0x00040005,
		0x0000000b,0x6c6f6366,0x0000726f,0x00040047,
		0x00000009,0x0000001e,0x00000000,0x00040047,
		0x0000000b,0x0000001e,0x00000000,0x00020013,
		0x00000002,0x00030021,0x00000003,0x00000002,
		0x00030016,0x00000006,0x00000020,0x00040017,
		0x00000007,0x00000006,0x00000004,0x00040020,
		0x00000008,0x00000003,0x00000007,0x0004003b,
		0x00000008,0x00000009,0x00000003,0x00040020,
		0x0000000a,0x00000001,0x00000007,0x0004003b,
		0x0000000a,0x0000000b,0x00000001,0x00050036,
		0x00000002,0x00000004,0x00000000,0x00000003,
		0x000200f8,0x00000005,0x0004003d,0x00000007,
		0x0000000c,0x0000000b,0x0003003e,0x00000009,
		0x0000000c,0x000100fd,0x00010038
	};

constexpr float vertex_data_pos[]
	{
		 1.f, -1.f, 0.f, 1.f,
		-1.f, -1.f, 0.f, 1.f,
		 0.f,  1.f, 0.f, 1.f,
	};
constexpr float vertex_data_col[]
	{
		1.f , 0.f , 0.f, 1.f,
		0.f , 1.f , 0.f, 1.f,
		0.f , 0.f , 1.f, 1.f,
	};

constexpr float vertex_data[]
	{
		vertex_data_pos[0], vertex_data_pos[1], vertex_data_pos[2] , vertex_data_pos[3] ,
		vertex_data_col[0], vertex_data_col[1], vertex_data_col[2] , vertex_data_col[3] ,
		vertex_data_pos[4], vertex_data_pos[5], vertex_data_pos[6] , vertex_data_pos[7] ,
		vertex_data_col[4], vertex_data_col[5], vertex_data_col[6] , vertex_data_col[7] ,
		vertex_data_pos[8], vertex_data_pos[9], vertex_data_pos[10], vertex_data_pos[11],
		vertex_data_col[8], vertex_data_col[9], vertex_data_col[10], vertex_data_col[11],
	};

namespace psl::webgpu
{
	inline namespace
	{
		template<typename T>
		class wgpu_resource_t
		{
		  public:
			constexpr auto resource() const noexcept -> const T& { return m_Resource; }
		  protected:
			constexpr wgpu_resource_t(T resource) noexcept : m_Resource(resource) {}
			constexpr wgpu_resource_t() noexcept = default;
			T m_Resource{};
		};
	}
	class device	:	public wgpu_resource_t<wgpu::Device>
	{
	  public:
	  	device()	:	wgpu_resource_t(wgpu::Device::Acquire(emscripten_webgpu_get_device()))
		{
			m_Resource.SetUncapturedErrorCallback(
				[](WGPUErrorType errorType, const char* message, void*) {
				printf("%d: %s\n", errorType, message);
				}, nullptr);
		}
	};

	class surface	:	public wgpu_resource_t<wgpu::Surface>
	{
	  public:
	  	surface()
		{
			wgpu::SurfaceDescriptorFromCanvasHTMLSelector canvasDesc{};
			canvasDesc.selector = "#canvas";

			wgpu::SurfaceDescriptor surfDesc{};
			surfDesc.nextInChain = &canvasDesc;

			wgpu::Instance instance{};  // null instance
			m_Resource = instance.CreateSurface(&surfDesc);
		}
	};

	class queue	:	public wgpu_resource_t<wgpu::Queue>
	{
	public:
		queue(device& device)	:	wgpu_resource_t(device.resource().GetDefaultQueue()) {}
	};
	
	class texture	:	public wgpu_resource_t<wgpu::Texture>
	{
		public:
		texture(device& device, size_t width, size_t height, size_t layers, wgpu::TextureFormat format, wgpu::TextureUsage usage)
		{
			wgpu::Extent3D dimensions{};
			dimensions.width  = width;
			dimensions.height = height;
			dimensions.depth  = layers;

			wgpu::TextureDescriptor desc{};
			desc.size   = dimensions;
			desc.usage  = usage;
			desc.format = format;

			m_Resource = device.resource().CreateTexture(&desc);
		}
	};
	struct with_depth_texture_t{};
	inline constexpr auto with_depth_texture = with_depth_texture_t{};

	class swapchain	:	public wgpu_resource_t<wgpu::SwapChain>
	{
	  public:
		swapchain(device& device, surface& surface, size_t width = 1280, size_t height = 720, wgpu::TextureFormat format = DEFAULT_SWAPCHAIN_FORMAT, wgpu::PresentMode presentMode = wgpu::PresentMode::Fifo, std::optional<wgpu::TextureFormat> depthFormat = std::nullopt) : 
			m_Device(device),
			m_Surface(surface),
			m_Width(width),
			m_Height(height),
			m_Format(format),
			m_DepthFormat(depthFormat.value_or(DEFAULT_DEPTH_FORMAT)),
			m_PresentMode(presentMode)
		{
			recreate_swapchain(depthFormat.has_value());
		}

		void resize(size_t width, size_t height) { m_Width = width; m_Height = height; recreate_swapchain(m_DepthTexture.has_value()); }
		constexpr auto width() const noexcept -> size_t { return m_Width; }
		constexpr auto height() const noexcept -> size_t { return m_Height; }

		auto depth_resource() const noexcept -> const auto& { return m_DepthTexture->resource(); }
	  private:

		void recreate_swapchain(bool withDepth)
		{
			wgpu::SwapChainDescriptor scDesc{};
			scDesc.usage       = wgpu::TextureUsage::RenderAttachment;
			scDesc.format      = m_Format;
			scDesc.width       = m_Width;
			scDesc.height      = m_Height;
			scDesc.presentMode = m_PresentMode;

			m_Resource = m_Device.resource().CreateSwapChain(m_Surface.resource(), &scDesc);
			if(withDepth)
			{
				m_DepthTexture = texture{m_Device, m_Width, m_Height, 1u, m_DepthFormat, wgpu::TextureUsage::RenderAttachment};
			}
		}
		device& m_Device;
		surface& m_Surface;
		std::optional<texture> m_DepthTexture { std::nullopt };
		wgpu::TextureFormat m_Format{DEFAULT_SWAPCHAIN_FORMAT};
		wgpu::TextureFormat m_DepthFormat{DEFAULT_DEPTH_FORMAT};
		wgpu::PresentMode m_PresentMode{wgpu::PresentMode::Fifo};
		size_t m_Width{1280};
		size_t m_Height{720};
	};

	enum class module_type
	{
		VERT,
		FRAG,
		COMPUTE,
	};

	class generic_module	:	public wgpu_resource_t<wgpu::ShaderModule>
	{
	  public:
		template<size_t N>
		generic_module(device& device, const uint32_t (&bytecode)[N], module_type type)  : m_Type(type)
		{
			wgpu::ShaderModuleSPIRVDescriptor spirvDesc{};
			spirvDesc.codeSize = N;
			spirvDesc.code     = bytecode;

			wgpu::ShaderModuleDescriptor descriptor{};
			descriptor.nextInChain = &spirvDesc;

			m_Resource = device.resource().CreateShaderModule(&descriptor);
		}
		
	  protected:
		module_type        m_Type; // unneeded, but good for debug logging
	};

	template<module_type Type>
	class module final	:	public generic_module
	{
	  public:
		module(device& device, const auto& bytecode)	:	generic_module(device, bytecode, Type) {}
	};

	using module_vert = module<module_type::VERT>;

	class generic_buffer	:	public wgpu_resource_t<wgpu::Buffer>
	{
	  public:
		generic_buffer(device& device, size_t size, wgpu::BufferUsage usage)
		{
			wgpu::BufferDescriptor descriptor{};
			descriptor.size             = size;
			descriptor.usage            = wgpu::BufferUsage::Vertex;
			descriptor.mappedAtCreation = true;

			m_Resource = device.resource().CreateBuffer(&descriptor);
		}

		template<size_t N>
		void map(const float (&data)[N])
		{
			map(data, N * sizeof(float));
		}

		void map(const float* data, size_t size)
		{
			auto mapped_range = m_Resource.GetMappedRange();
			memcpy(mapped_range, data, size);
			m_Resource.Unmap();
		}
	};

	enum class buffer_usage
	{
		VBO,
		IBO,
		UBO,
		SSBO,
	};

	inline namespace
	{
		constexpr auto to_wgpu(buffer_usage usage) -> wgpu::BufferUsage
		{
			switch(usage)
			{
				case buffer_usage::VBO: return wgpu::BufferUsage::Vertex;
				case buffer_usage::IBO: return wgpu::BufferUsage::Index;
				case buffer_usage::UBO: return wgpu::BufferUsage::Uniform;
				case buffer_usage::SSBO: return wgpu::BufferUsage::Storage;
			}
			throw std::exception();
			return wgpu::BufferUsage::None;
		}
	}

	template<buffer_usage Usage>
	class buffer	:	public generic_buffer
	{
	  public:
		buffer(device& device, size_t size) : generic_buffer(device, size, to_wgpu(Usage)){}
	};

	class pipeline	:	public wgpu_resource_t<wgpu::RenderPipeline>
	{
		public:
		pipeline(device& device, module<module_type::VERT>& vertModule, module<module_type::FRAG>& fragModule)
		{
			TODO("pipeline is currently hardcoded, good for fast example, bad for abstraction.");

			wgpu::ProgrammableStageDescriptor fragmentStage{};
			fragmentStage.module = fragModule.resource();
			fragmentStage.entryPoint = "main";

			wgpu::ProgrammableStageDescriptor vertexStage{};
			vertexStage.module = vertModule.resource();
			vertexStage.entryPoint = "main";
			
			wgpu::ColorStateDescriptor colorStateDescriptor{};
			colorStateDescriptor.format = DEFAULT_SWAPCHAIN_FORMAT;

			wgpu::DepthStencilStateDescriptor depthstencilStateDescriptor{};
			depthstencilStateDescriptor.format            = DEFAULT_DEPTH_FORMAT;
			depthstencilStateDescriptor.depthCompare      = wgpu::CompareFunction::Less;
			depthstencilStateDescriptor.depthWriteEnabled = true;

			wgpu::PipelineLayoutDescriptor pl{};
			pl.bindGroupLayoutCount = 0;
			pl.bindGroupLayouts = nullptr;
			
			// describes attribute layout
			wgpu::VertexStateDescriptor vertexDescr{};
			wgpu::VertexAttributeDescriptor vertAttributes[2]{};
			wgpu::VertexBufferLayoutDescriptor vertexAttrDescr{};

			vertAttributes[0].format         = wgpu::VertexFormat::Float4;
			vertAttributes[0].offset         = 0;
			vertAttributes[0].shaderLocation = 0;			
			vertAttributes[1].format         = wgpu::VertexFormat::Float4;
			vertAttributes[1].offset         = 4 * 4; // 4 floats of 32 bits.
			vertAttributes[1].shaderLocation = 1;

			vertexAttrDescr.attributeCount = 2;
			vertexAttrDescr.attributes = vertAttributes;
			vertexAttrDescr.arrayStride = 2 * (4 * 4); // 2 attributes of float4 (32 bits per float)

			vertexDescr.vertexBufferCount = 1;
			vertexDescr.vertexBuffers = &vertexAttrDescr;	

			wgpu::RenderPipelineDescriptor descriptor{};
			descriptor.layout = device.resource().CreatePipelineLayout(&pl);
			descriptor.vertexStage = vertexStage;
			descriptor.vertexState = &vertexDescr;
			descriptor.fragmentStage = &fragmentStage;
			descriptor.colorStateCount = 1;
			descriptor.colorStates = &colorStateDescriptor;
			descriptor.primitiveTopology = wgpu::PrimitiveTopology::TriangleList;
			descriptor.depthStencilState = &depthstencilStateDescriptor;
			m_Resource = device.resource().CreateRenderPipeline(&descriptor);
		}
	};
}

std::unique_ptr<psl::webgpu::device>                                  m_Device        = nullptr;
std::unique_ptr<psl::webgpu::surface>                                 m_Surface       = nullptr;
std::unique_ptr<psl::webgpu::swapchain>                               m_SwapChain     = nullptr;
std::unique_ptr<psl::webgpu::pipeline>                                m_Pipeline      = nullptr;
std::unique_ptr<psl::webgpu::buffer<psl::webgpu::buffer_usage::VBO>>  m_VertexBuffer  = nullptr;
std::unique_ptr<psl::webgpu::queue>                                   m_Queue         = nullptr;

void render_loop()
{
	TODO_ONCE("use renderbuffer to cache draw instruction/renderpass setup");
	wgpu::RenderPassDescriptor descr{};
	wgpu::RenderPassColorAttachmentDescriptor swapchainColorAttachment{};
	swapchainColorAttachment.attachment = m_SwapChain->resource().GetCurrentTextureView();
	swapchainColorAttachment.clearColor = { 0.6, 0.6, 0.6, 1.0 };
	swapchainColorAttachment.storeOp    = wgpu::StoreOp::Store;

	
	wgpu::RenderPassDepthStencilAttachmentDescriptor swapchainDepthStencilAttachment{};
	swapchainDepthStencilAttachment.attachment      = m_SwapChain->depth_resource().CreateView();
	swapchainDepthStencilAttachment.clearDepth     = 1.0;
	swapchainDepthStencilAttachment.depthStoreOp   = wgpu::StoreOp::Store;
	swapchainDepthStencilAttachment.clearStencil   = 0;
	swapchainDepthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Clear;

	descr.colorAttachments       = &swapchainColorAttachment;
	descr.colorAttachmentCount   = 1;
	descr.depthStencilAttachment = &swapchainDepthStencilAttachment;

	auto commandEncoder = m_Device->resource().CreateCommandEncoder();
	auto renderPass = commandEncoder.BeginRenderPass(&descr);

	{ // actual drawing of objects happen here
		renderPass.SetPipeline(m_Pipeline->resource());
		renderPass.SetVertexBuffer(0, m_VertexBuffer->resource());
		renderPass.Draw(3, 1, 0, 0);
	}

	renderPass.EndPass();
	auto renderBuffer = commandEncoder.Finish();
	m_Queue->resource().Submit(1, &renderBuffer);
}

void resize_swapchain()
{
	double width{}, height{};
	emscripten_get_element_css_size("#canvas", &width, &height);
	m_SwapChain->resize(width, height);
}

EMSCRIPTEN_BINDINGS(Module)
{
	emscripten::function("resize_swapchain", &resize_swapchain);
}

int main()
{
	m_Device    = std::make_unique<psl::webgpu::device>();
	m_Surface   = std::make_unique<psl::webgpu::surface>();
	double width{}, height{};
	emscripten_get_element_css_size("#canvas", &width, &height);
	m_SwapChain = std::make_unique<psl::webgpu::swapchain>(*m_Device, *m_Surface, width, height, DEFAULT_SWAPCHAIN_FORMAT, wgpu::PresentMode::Fifo, DEFAULT_DEPTH_FORMAT);

	auto vertModule = psl::webgpu::module<psl::webgpu::module_type::VERT>{*m_Device, vert_spv};
	auto fragModule = psl::webgpu::module<psl::webgpu::module_type::FRAG>{*m_Device, frag_spv};

	m_Pipeline     = std::make_unique<psl::webgpu::pipeline>(*m_Device, vertModule, fragModule);
	m_VertexBuffer = std::make_unique<psl::webgpu::buffer<psl::webgpu::buffer_usage::VBO>>(*m_Device, sizeof(vertex_data) * sizeof(vertex_data[0]));
	m_VertexBuffer->map(vertex_data);

	m_Queue = std::make_unique<psl::webgpu::queue>(*m_Device);

    emscripten_set_main_loop(render_loop, 0, false);

	return 0;
}
