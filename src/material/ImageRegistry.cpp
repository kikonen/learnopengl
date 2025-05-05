#include "imageRegistry.h"

#include "material/ImageTexture.h"
#include "material/Image.h"

namespace {
    thread_local std::exception_ptr lastException = nullptr;

    static ImageRegistry* s_registry{ nullptr };
}

void ImageRegistry::init() noexcept
{
    assert(!s_registry);
    s_registry = new ImageRegistry();
}

void ImageRegistry::release() noexcept
{
    auto* s = s_registry;
    s_registry = nullptr;
    delete s;
}

ImageRegistry& ImageRegistry::get() noexcept
{
    assert(s_registry);
    return *s_registry;
}

ImageRegistry::ImageRegistry()
{
    clear();
}

ImageRegistry::~ImageRegistry()
{
    clear();
}

void ImageRegistry::clear()
{
    {
        std::lock_guard lock(m_lock);
        for (auto& future : m_textures) {
            // TODO KI somehow abort
        }
        m_textures.clear();
    }
}

std::shared_future<std::shared_ptr<ImageTexture>> ImageRegistry::getTexture(
    std::string_view name,
    std::string_view path,
    bool grayScale,
    bool gammaCorrect,
    bool flipY,
    const TextureSpec& spec)
{
    const std::string cacheKey = fmt::format(
        "{}_{}_{}-{}_{}-{}_{}_{}",
        path,
        grayScale,
        gammaCorrect,
        flipY,
        spec.wrapS, spec.wrapT,
        spec.minFilter, spec.magFilter, spec.mipMapLevels);

    std::lock_guard lock(m_lock);
    {
        const auto& e = m_textures.find(cacheKey);
        if (e != m_textures.end())
            return e->second;
    }

    auto future = startLoad(
        std::make_shared<ImageTexture>(name, path, grayScale, gammaCorrect, flipY, spec));

    m_textures[cacheKey] = future;

    return future;
}

std::shared_future<std::shared_ptr<ImageTexture>> ImageRegistry::startLoad(
    std::shared_ptr<ImageTexture> texture)
{
    /*
std::future<int> spawn_async_task(){
std::promise<int> p;
std::thread t([p=std::move(p)](){ p.set_value(find_the_answer());});
t.detach();
return f;
}        */
    std::promise<std::shared_ptr<ImageTexture>> promise;
    auto future = promise.get_future().share();

    // NOTE KI use thread instead of std::async since std::future blocking/cleanup is problematic
    // https://stackoverflow.com/questions/21531096/can-i-use-stdasync-without-waiting-for-the-future-limitation
    auto th = std::thread{
        [texture, p = std::move(promise)]() mutable {
            try {
               texture->load();
               p.set_value(texture);
            }
            catch (const std::exception& ex) {
                KI_CRITICAL(fmt::format("IMAGE_TEX_ERROR: {}", ex.what()));
                lastException = std::current_exception();
                p.set_exception(lastException);
            }
            catch (const std::string& ex) {
                KI_CRITICAL(fmt::format("IMAGE_TEX_ERROR: {}", ex));
                lastException = std::current_exception();
                p.set_exception(lastException);
            }
            catch (const char* ex) {
                KI_CRITICAL(fmt::format("IMAGE_TEX_ERROR: {}", ex));
                lastException = std::current_exception();
                p.set_exception(lastException);
            }
            catch (...) {
                KI_CRITICAL(fmt::format("IMAGE_TEX_ERROR: {}", "UNKNOWN_ERROR"));
                lastException = std::current_exception();
                p.set_exception(lastException);
            }
        }
    };
    th.detach();

    return future;
}

