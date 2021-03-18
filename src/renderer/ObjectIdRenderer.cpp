#include "ObjectIdRenderer.h"


ObjectIdRenderer::ObjectIdRenderer(const Assets& assets)
	: Renderer(assets)
{
	idShader = Shader::getShader(assets, TEX_OBJECT_ID);
}

int ObjectIdRenderer::getObjectId(const RenderContext& ctx, double screenPosX, double screenPosY, Viewport* mainViewport)
{
	// https://stackoverflow.com/questions/10123601/opengl-read-pixels-from-framebuffer-for-ing-rounded-up-to-255-0xff
	// https://stackoverflow.com/questions/748162/what-are-the-differences-between-a-frame-buffer-object-and-a-pixel-buffer-object

	glFlush();
	glFinish();


	unsigned char data[4];
	memset(data, 100, sizeof(data));

	int screenW = ctx.width;
	int screenH = ctx.height;

	float w = screenW * (mainViewport->size.x / 2.f);
	float h = screenH * (mainViewport->size.y / 2.f);

	float ratioX = idBuffer->spec.width / w;
	float ratioY = idBuffer->spec.height / h;

	float offsetX = screenW * (mainViewport->pos.x + 1.f) / 2.f;
	float offsetY = screenH * (1.f - (mainViewport->pos.y + 1.f) / 2.f);

	float posx = (screenPosX - offsetX) * ratioX;
	float posy = (screenPosY - offsetY) * ratioY;

	if (posx < 0 || posx > w || posy < 0 || posy > h) return -1;


	{
		idBuffer->bind(ctx);

		glReadBuffer(GL_COLOR_ATTACHMENT0);

		int readFormat;
		glGetFramebufferParameteriv(GL_FRAMEBUFFER, GL_IMPLEMENTATION_COLOR_READ_FORMAT, &readFormat);

		glReadPixels(posx, idBuffer->spec.height - posy, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);

		int x = 0;
		idBuffer->unbind(ctx);

	}

	int objectID =
		data[0] +
		data[1] * 256 +
		data[2] * 256 * 256;

	return objectID;
}

void ObjectIdRenderer::prepare()
{
	idShader->prepare();

	debugViewport = new Viewport(
		glm::vec3(-1.0, -0.5, 0),
		glm::vec3(0, 0, 0),
		glm::vec2(0.5f, 0.5f),
		-1,
		Shader::getShader(assets, TEX_VIEWPORT));

	debugViewport->prepare();
}

void ObjectIdRenderer::update(const RenderContext& ctx, NodeRegistry& registry)
{
	if (idBuffer) return;
	// https://riptutorial.com/opengl/example/28872/using-pbos

	idBuffer = new TextureBuffer({ ctx.width, ctx.height, { FrameBufferAttachment::getObjectId(), FrameBufferAttachment::getRBODepth() } });
	idBuffer->prepare();
	debugViewport->setTextureID(idBuffer->spec.attachments[0].textureID);
}

void ObjectIdRenderer::bind(const RenderContext& ctx)
{
}

void ObjectIdRenderer::render(const RenderContext& ctx, NodeRegistry& registry)
{
	RenderContext idCtx(ctx.assets, ctx.clock, ctx.state, ctx.scene, ctx.camera, idBuffer->spec.width, idBuffer->spec.height);
	idCtx.lightSpaceMatrix = ctx.lightSpaceMatrix;

	idBuffer->bind(idCtx);
	drawNodes(idCtx, registry);
	idBuffer->unbind(ctx);
}

void ObjectIdRenderer::drawNodes(const RenderContext& ctx, NodeRegistry& registry)
{
	ctx.state.enable(GL_DEPTH_TEST);

	// https://cmichel.io/understanding-front-faces-winding-order-and-normals
	ctx.state.enable(GL_CULL_FACE);
	ctx.state.cullFace(GL_BACK);
	ctx.state.frontFace(GL_CCW);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (auto& x : registry.nodes) {
		NodeType* t = x.first;
		t->bind(ctx, idShader);

		Batch& batch = t->batch;
		batch.objectId = true;
		batch.bind(ctx, idShader);

		for (auto& e : x.second) {
			batch.draw(ctx, e, idShader);
		}

		batch.flush(ctx, t);
		batch.objectId = false;
	}
}
