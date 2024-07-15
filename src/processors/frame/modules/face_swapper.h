/**
 ******************************************************************************
 * @file           : face_swapper.h
 * @author         : CodingRookie
 * @brief          : None
 * @attention      : None
 * @date           : 24-7-6
 ******************************************************************************
 */

#ifndef FACEFUSIONCPP_SRC_PROCESSORS_FRAME_MODULES_FACE_SWAPPER_H_
#define FACEFUSIONCPP_SRC_PROCESSORS_FRAME_MODULES_FACE_SWAPPER_H_

#include <onnxruntime_cxx_api.h>
#include <onnx/onnx_pb.h>
#include <fstream>
#include <opencv2/opencv.hpp>
#include "nlohmann/json.hpp"
#include "vision.h"
#include "typing.h"
#include "face_analyser/face_analyser.h"
#include "globals.h"
#include "face_masker.h"
#include "ort_session.h"

namespace Ffc {

class FaceSwapper : public OrtSession {
public:
    explicit FaceSwapper(const std::shared_ptr<Ort::Env> &env,
                         const std::shared_ptr<FaceAnalyser> &faceAnalyser,
                         const std::shared_ptr<FaceMasker> &faceMasker,
                         const std::shared_ptr<nlohmann::json> &modelsInfoJson);
    ~FaceSwapper() = default;

    void processImage(const std::vector<std::string> &sourcePaths,
                      const std::string &targetPath,
                      const std::string &outputPath);
    void setFaceAnalyser(const std::shared_ptr<FaceAnalyser> &faceAnalyser);

private:
    std::shared_ptr<Typing::VisionFrame> processFrame(const Typing::Faces &referenceFaces,
                                                      const Typing::Face &sourceFace,
                                                      const Typing::VisionFrame &targetFrame);
    std::shared_ptr<Typing::VisionFrame> swapFace(const Typing::Face &sourceFace, const Typing::Face &targetFace,
                                                  const Typing::VisionFrame &targetFrame);
    std::shared_ptr<Typing::VisionFrame> applySwap(const Typing::Face &sourceFace,
                                                   const Typing::Face &targetFace,
                                                   const Typing::VisionFrame &targetFrame);
    // 初始化部分变量
    void init();
    // 标准化，归一化, 返回BGR
    static std::shared_ptr<Typing::VisionFrame>
    prepareCropVisionFrame(const Typing::VisionFrame &visionFrame,
                           const std::vector<float> &mean,
                           const std::vector<float> &standDeviation);
    std::shared_ptr<std::vector<cv::Mat>>
    getCropMasks(const Typing::VisionFrame &visionFrame, const cv::Size &cropSize,
                 const float &faceMaskBlur, const Typing::Padding &faceMaskPadding);
    std::shared_ptr<std::vector<float>> prepareSourceEmbedding(const Typing::Face &sourceFace);
    std::shared_ptr<std::vector<float>> prepareCropFrameData(const Typing::VisionFrame &cropFrame) const;

    std::shared_ptr<FaceAnalyser> m_faceAnalyser = nullptr;
    std::shared_ptr<FaceMasker> m_faceMasker = nullptr;
    const std::shared_ptr<nlohmann::json> m_modelsInfoJson;
    std::shared_ptr<Typing::EnumFaceSwapperModel> m_faceSwapperModel = nullptr;
    int m_inputHeight;
    int m_inputWidth;
    cv::Size m_size;
    std::vector<float> m_mean;
    std::vector<float> m_standardDeviation;
    std::vector<cv::Point2f> m_warpTemplate;
    std::vector<float> m_initializerArray;
    std::string m_modelName;
};

} // namespace Ffc

#endif // FACEFUSIONCPP_SRC_PROCESSORS_FRAME_MODULES_FACE_SWAPPER_H_
