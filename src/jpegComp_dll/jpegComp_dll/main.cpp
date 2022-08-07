#include <lua.hpp>
#include <vector>
using namespace std;

#define min(a, b) ((a < b) ? a : b)
#define max(a, b) ((a > b) ? a : b)
#define clip(x, a, b) (min(max(x, a), b))

struct Pixel_RGBA {
    unsigned char b;
    unsigned char g;
    unsigned char r;
    unsigned char a;
};

// u = Cr, v = Cb
struct intPixel_YUVA {
    int v;
    int u;
    int y;
    int a;

public:
    intPixel_YUVA operator +(intPixel_YUVA q) {
        intPixel_YUVA toReturn;
        toReturn.v = this->v + q.v;
        toReturn.u = this->u + q.u;
        toReturn.y = this->y + q.y;
        toReturn.a = this->a + q.a;
        return toReturn;
    }

    intPixel_YUVA operator -(intPixel_YUVA q) {
        intPixel_YUVA toReturn;
        toReturn.v = this->v - q.v;
        toReturn.u = this->u - q.u;
        toReturn.y = this->y - q.y;
        toReturn.a = this->a - q.a;
        return toReturn;
    }

    intPixel_YUVA operator *(float r) {
        intPixel_YUVA toReturn;
        toReturn.v = this->v * r;
        toReturn.u = this->u * r;
        toReturn.y = this->y * r;
        toReturn.a = this->a * r;
        return toReturn;
    }

    intPixel_YUVA operator /(float r) {
        intPixel_YUVA toReturn;
        toReturn.v = this->v / r;
        toReturn.u = this->u / r;
        toReturn.y = this->y / r;
        toReturn.a = this->a / r;
        return toReturn;
    }
};

const float sq05 = 0.7071067811865476F;
const float sq2 = 1.4142135623730951F;
const float C4 = 0.7071067811865476F;
const float C8[2] = { 0.541196100146197F, 1.3065629648763764F };
const float C16[4] = { 0.5097955791041592F, 0.6013448869350453F, 0.8999762231364156F, 2.5629154477415055F };

const int Q_Y[64] = { 16, 11, 10, 16, 24, 40, 51, 61,
                      12, 12, 14, 19, 26, 58, 60, 55,
                      14, 13, 16, 24, 40, 57, 69, 55,
                      14, 17, 22, 29, 51, 87, 80, 62,
                      18, 22, 37, 56, 68, 109, 103, 77,
                      24, 35, 55, 64, 81, 104, 113, 92,
                      49, 64, 78, 87, 103, 121, 120, 101,
                      72, 92, 95, 98, 112, 100, 103, 99 };

const int Q_UV[64] = { 17, 18, 24, 47, 99, 99, 99, 99,
                       18, 21, 26, 66, 99, 99, 99, 99,
                       24, 26, 56, 99, 99, 99, 99, 99,
                       47, 66, 99, 99, 99, 99, 99, 99,
                       99, 99, 99, 99, 99, 99, 99, 99,
                       99, 99, 99, 99, 99, 99, 99, 99,
                       99, 99, 99, 99, 99, 99, 99, 99,
                       99, 99, 99, 99, 99, 99, 99, 99 };

std::vector<intPixel_YUVA> xFDCT2(std::vector<intPixel_YUVA> pixels) {
    std::vector<intPixel_YUVA> freqs(2);

    freqs[0] = pixels[0] + pixels[1];
    freqs[1] = (pixels[0] - pixels[1]) * C4;

    return freqs;
}

std::vector<intPixel_YUVA> xFDCT4(std::vector<intPixel_YUVA> pixels) {
    std::vector<intPixel_YUVA> freqs(4), g(2), h(2), G(2), H(2);

    g[0] = pixels[0] + pixels[3];
    g[1] = pixels[1] + pixels[2];

    h[0] = (pixels[0] - pixels[3]) * C8[0];
    h[1] = (pixels[1] - pixels[2]) * C8[1];

    G = xFDCT2(g);
    H = xFDCT2(h);

    freqs[0] = G[0];
    freqs[2] = G[1];
    freqs[1] = H[0] + H[1];
    freqs[3] = H[1];

    return freqs;
}

std::vector<intPixel_YUVA> xFDCT8(std::vector<intPixel_YUVA> pixels, int i) {
    std::vector<intPixel_YUVA> freqs(8), g(4), h(4), G(4), H(4);

    g[0] = pixels[i] + pixels[i + 7];
    g[1] = pixels[i + 1] + pixels[i + 6];
    g[2] = pixels[i + 2] + pixels[i + 5];
    g[3] = pixels[i + 3] + pixels[i + 4];

    h[0] = (pixels[i] - pixels[i + 7]) * C16[0];
    h[1] = (pixels[i + 1] - pixels[i + 6]) * C16[1];
    h[2] = (pixels[i + 2] - pixels[i + 5]) * C16[2];
    h[3] = (pixels[i + 3] - pixels[i + 4]) * C16[3];

    G = xFDCT4(g);
    H = xFDCT4(h);

    freqs[0] = G[0];
    freqs[2] = G[1];
    freqs[4] = G[2];
    freqs[6] = G[3];
    freqs[1] = H[0] + H[1];
    freqs[3] = H[1] + H[2];
    freqs[5] = H[2] + H[3];
    freqs[7] = H[3];

    return freqs;
}

std::vector<intPixel_YUVA> yFDCT8(std::vector<intPixel_YUVA> pixels, int i, int w) {
    std::vector<intPixel_YUVA> freqs(8), g(4), h(4), G(4), H(4);

    g[0] = pixels[i] + pixels[i + 7 * w];
    g[1] = pixels[i + 1 * w] + pixels[i + 6 * w];
    g[2] = pixels[i + 2 * w] + pixels[i + 5 * w];
    g[3] = pixels[i + 3 * w] + pixels[i + 4 * w];

    h[0] = (pixels[i] - pixels[i + 7 * w]) * C16[0];
    h[1] = (pixels[i + 1 * w] - pixels[i + 6 * w]) * C16[1];
    h[2] = (pixels[i + 2 * w] - pixels[i + 5 * w]) * C16[2];
    h[3] = (pixels[i + 3 * w] - pixels[i + 4 * w]) * C16[3];

    G = xFDCT4(g);
    H = xFDCT4(h);

    freqs[0] = G[0];
    freqs[2] = G[1];
    freqs[4] = G[2];
    freqs[6] = G[3];
    freqs[1] = H[0] + H[1];
    freqs[3] = H[1] + H[2];
    freqs[5] = H[2] + H[3];
    freqs[7] = H[3];

    return freqs;
}

std::vector<intPixel_YUVA> xIDCT2(std::vector<intPixel_YUVA> freqs) {
    std::vector<intPixel_YUVA> pixels(2);

    pixels[0] = freqs[0] / 2 + freqs[1] * C4;
    pixels[1] = freqs[0] / 2 - freqs[1] * C4;

    return pixels;
}

std::vector<intPixel_YUVA> xIDCT4(std::vector<intPixel_YUVA> freqs) {
    std::vector<intPixel_YUVA> pixels(4), g(2), h(2), G(2), H(2);

    G[0] = freqs[0];
    G[1] = freqs[2];

    H[0] = freqs[1] * 2;
    H[1] = freqs[3] + freqs[1];

    g = xIDCT2(G);
    h = xIDCT2(H);

    h[0] = h[0] * C8[0];
    h[1] = h[1] * C8[1];

    pixels[0] = (g[0] + h[0]) / 2;
    pixels[1] = (g[1] + h[1]) / 2;
    pixels[2] = (g[1] - h[1]) / 2;
    pixels[3] = (g[0] - h[0]) / 2;

    return pixels;
}

std::vector<intPixel_YUVA> xIDCT8(std::vector<intPixel_YUVA> freqs, int i) {
    std::vector<intPixel_YUVA> pixels(8), g(4), h(4), G(4), H(4);

    G[0] = freqs[i];
    G[1] = freqs[i + 2];
    G[2] = freqs[i + 4];
    G[3] = freqs[i + 6];

    H[0] = freqs[i + 1] * 2;
    H[1] = freqs[i + 3] + freqs[i + 1];
    H[2] = freqs[i + 5] + freqs[i + 3];
    H[3] = freqs[i + 7] + freqs[i + 5];

    g = xIDCT4(G);
    h = xIDCT4(H);

    h[0] = h[0] * C16[0];
    h[1] = h[1] * C16[1];
    h[2] = h[2] * C16[2];
    h[3] = h[3] * C16[3];

    pixels[0] = (g[0] + h[0]) / 2;
    pixels[1] = (g[1] + h[1]) / 2;
    pixels[2] = (g[2] + h[2]) / 2;
    pixels[3] = (g[3] + h[3]) / 2;
    pixels[4] = (g[3] - h[3]) / 2;
    pixels[5] = (g[2] - h[2]) / 2;
    pixels[6] = (g[1] - h[1]) / 2;
    pixels[7] = (g[0] - h[0]) / 2;

    return pixels;
}

std::vector<intPixel_YUVA> yIDCT8(std::vector<intPixel_YUVA> freqs, int i, int w) {
    std::vector<intPixel_YUVA> pixels(8), g(4), h(4), G(4), H(4);

    G[0] = freqs[i];
    G[1] = freqs[i + w * 2];
    G[2] = freqs[i + w * 4];
    G[3] = freqs[i + w * 6];

    H[0] = freqs[i + w] * 2;
    H[1] = freqs[i + w * 3] + freqs[i + w];
    H[2] = freqs[i + w * 5] + freqs[i + w * 3];
    H[3] = freqs[i + w * 7] + freqs[i + w * 5];

    g = xIDCT4(G);
    h = xIDCT4(H);

    h[0] = h[0] * C16[0];
    h[1] = h[1] * C16[1];
    h[2] = h[2] * C16[2];
    h[3] = h[3] * C16[3];

    pixels[0] = (g[0] + h[0]) / 2;
    pixels[1] = (g[1] + h[1]) / 2;
    pixels[2] = (g[2] + h[2]) / 2;
    pixels[3] = (g[3] + h[3]) / 2;
    pixels[4] = (g[3] - h[3]) / 2;
    pixels[5] = (g[2] - h[2]) / 2;
    pixels[6] = (g[1] - h[1]) / 2;
    pixels[7] = (g[0] - h[0]) / 2;

    return pixels;
}


int jpegComp(lua_State* L) {
    Pixel_RGBA* pixels = reinterpret_cast<Pixel_RGBA*>(lua_touserdata(L, 1));
    int w = static_cast<int>(lua_tointeger(L, 2));
    int h = static_cast<int>(lua_tointeger(L, 3));
    double q = static_cast<double>(lua_tonumber(L, 4));
    bool comp_alpha = static_cast<bool>(lua_toboolean(L, 5));

    int padded_w = (w % 8 == 0) ? w : w + 8 - (w % 8);
    int padded_h = (h % 8 == 0) ? h : h + 8 - (h % 8);

    std::vector<intPixel_YUVA> int_pixels(padded_w * padded_h);
    std::vector<intPixel_YUVA> int_freqs(8 * 8);

    // pixels (unchar RGB) -> int_pixels (YUV) に変換
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int index = x + w * y;
            int pindex = x + padded_w * y;

            int b = pixels[index].b;
            int g = pixels[index].g;
            int r = pixels[index].r;

            // ダウンサンプリングも並行
            int_pixels[pindex].y =   0.299F * r + 0.587F * g + 0.114F * b;
            int_pixels[pindex].u = ( 0.500F * r - 0.419F * g - 0.081F * b + 128) / 2;
            int_pixels[pindex].v = (-0.169F * r - 0.332F * g + 0.500F * b + 128) / 2;
            int_pixels[pindex].a = static_cast<int>(pixels[index].a);
        }
    }

    // padding
    for (int y = 0; y < h; y++) {
        for (int x = w; x < padded_w; x++) {
            int index = x + padded_w * y;
            int_pixels[index] = int_pixels[w * (y + 1) - 1];
        }
    }

    for (int y = h; y < padded_h; y++) {
        for (int x = 0; x < padded_w; x++) {
            int index = x + padded_w * y;
            int_pixels[index] = int_pixels[x + w * (h - 1)];
        }
    }

    // 量子化行列の設定
    int S_Y[64], S_UV[64];
    for (int i = 0; i < 64; i++) {
        S_Y[i] = 0.02 * (1 - static_cast<double>(Q_Y[i])) * q + 2 * static_cast<double>(Q_Y[i]) - 1;
        S_UV[i] = 0.02 * (1 - static_cast<double>(Q_UV[i])) * q + 2 * static_cast<double>(Q_UV[i]) - 1;
    }

    // FDCT, 量子化, 逆量子化, IDCT
    for (int y0 = 0; y0 < h; y0 += 8) {
        for (int x0 = 0; x0 < w; x0 += 8) {
            int index = x0 + padded_w * y0;

            // FDCT
            // y 軸に平行な FDCT
            for (int dx = 0; dx < 8; dx++) {
                std::vector<intPixel_YUVA> yfreqs(8);
                yfreqs = yFDCT8(int_pixels, index + dx, padded_w);
                for (int dy = 0; dy < 8; dy++) {
                    int_freqs[dx + 8 * dy] = yfreqs[dy];
                }
            }
            // x 軸に平行な FDCT
            for (int dy = 0; dy < 8; dy++) {
                std::vector<intPixel_YUVA> xfreqs(8);
                xfreqs = xFDCT8(int_freqs, 8 * dy);
                for (int dx = 0; dx < 8; dx++) {
                    int_freqs[dx + 8 * dy] = xfreqs[dx];
                }
            }

            // 量子化, 逆量子化
            for (int i = 0; i < 64; i++) {
                int_freqs[i].v /= S_UV[i];
                int_freqs[i].u /= S_UV[i];
                int_freqs[i].y /= S_Y[i];
                int_freqs[i].a /= S_Y[i];

                int_freqs[i].v *= S_UV[i];
                int_freqs[i].u *= S_UV[i];
                int_freqs[i].y *= S_Y[i];
                int_freqs[i].a *= S_Y[i];
            }


            // IDCT
            // x 軸に平行な IDCT
            for (int dx = 0; dx < 8; dx++) {
                std::vector<intPixel_YUVA> ypixels(8);
                ypixels = yIDCT8(int_freqs, dx, 8);
                for (int dy = 0; dy < 8; dy++) {
                    int_freqs[dx + 8 * dy] = ypixels[dy];
                }
            }

            // y 軸に平行な IDCT
            for (int dy = 0; dy < 8; dy++) {
                std::vector<intPixel_YUVA> xpixels(8);
                xpixels = xIDCT8(int_freqs, 8 * dy);
                for (int dx = 0; dx < 8; dx++) {
                    int_pixels[index + dx + padded_w * dy] = xpixels[dx];
                }
            }

        }
    }

    // int_pixels (YUV) -> pixels (uchar RGB) に変換
    if (comp_alpha) {
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                int index = x + w * y;
                int pindex = x + padded_w * y;

                // オーバーサンプリングも並行
                int v = 2 * int_pixels[pindex].v - 128;
                int u = 2 * int_pixels[pindex].u - 128;
                int y = int_pixels[pindex].y;

                int r = clip(y + 1.402F * u, 0, 255);
                int g = clip(y - 0.714F * u - 0.344F * v, 0, 255);
                int b = clip(y + 1.772F * v, 0, 255);

                pixels[index].r = static_cast<unsigned char>(r);
                pixels[index].g = static_cast<unsigned char>(g);
                pixels[index].b = static_cast<unsigned char>(b);

                pixels[index].r = static_cast<unsigned char>(r);
                pixels[index].g = static_cast<unsigned char>(g);
                pixels[index].b = static_cast<unsigned char>(b);
                pixels[index].a = static_cast<unsigned char>(clip(int_pixels[pindex].a, 0, 255));
            }
        }
    }
    else {
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                int index = x + w * y;
                int pindex = x + padded_w * y;

                // オーバーサンプリングも並行
                int v = 2 * int_pixels[pindex].v - 128;
                int u = 2 * int_pixels[pindex].u - 128;
                int y = int_pixels[pindex].y;

                int r = clip(y + 1.402F * u, 0, 255);
                int g = clip(y - 0.714F * u - 0.344F * v, 0, 255);
                int b = clip(y + 1.772F * v, 0, 255);

                pixels[index].r = static_cast<unsigned char>(r);
                pixels[index].g = static_cast<unsigned char>(g);
                pixels[index].b = static_cast<unsigned char>(b);
            }
        }
    }

	return 0;
}

static luaL_Reg functions[] = {
    {"jpegComp", jpegComp},
    {nullptr, nullptr}
};

extern "C" {
    __declspec(dllexport) int luaopen_jpegComp(lua_State* L) {
        luaL_register(L, "jpegComp", functions);
        return 1;
    }
}