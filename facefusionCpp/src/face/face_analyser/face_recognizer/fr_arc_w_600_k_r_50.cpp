/**
 ******************************************************************************
 * @file           : fr_arc_w_600_k_r_50.cpp
 * @author         : CodingRookie
 * @brief          : None
 * @attention      : None
 * @date           : 24-10-14
 ******************************************************************************
 */

#include "fr_arc_w_600_k_r_50.h"
#include "face_helper.h"

FRArcW600kR50::FRArcW600kR50(const std::shared_ptr<Ort::Env> &env, const std::string &modelPath) :
    FaceRecognizerBase(env, modelPath) {
    m_inputWidth = (int)m_inferenceSession.m_inputNodeDims[0][2];
    m_inputHeight = (int)m_inferenceSession.m_inputNodeDims[0][3];
}

std::vector<float> FRArcW600kR50::preProcess(const cv::Mat &visionFrame, const Face::Landmark &faceLandmark5_68) {
    std::vector<cv::Point2f> warpTemplate = FaceHelper::getWarpTemplate(FaceHelper::WarpTemplateType::Arcface_112_v2);
    cv::Mat cropVisionFrame;
    std::tie(cropVisionFrame, std::ignore) = FaceHelper::warpFaceByFaceLandmarks5(visionFrame, faceLandmark5_68,
                                                                                  warpTemplate,
                                                                                  cv::Size(112, 112));
    std::vector<cv::Mat> bgrChannels(3);
    split(cropVisionFrame, bgrChannels);
    for (int c = 0; c < 3; c++) {
        bgrChannels[c].convertTo(bgrChannels[c], CV_32FC1, 1 / 127.5, -1.0);
    }

    const int imageArea = this->m_inputHeight * this->m_inputWidth;
    std::vector<float> inputData;
    inputData.resize(3 * imageArea);
    size_t singleChnSize = imageArea * sizeof(float);
    memcpy(inputData.data(), (float *)bgrChannels[2].data, singleChnSize);
    memcpy(inputData.data() + imageArea, (float *)bgrChannels[1].data, singleChnSize);
    memcpy(inputData.data() + imageArea * 2, (float *)bgrChannels[0].data, singleChnSize);
    return inputData;
}

std::array<std::vector<float>, 2> FRArcW600kR50::recognize(const cv::Mat &visionFrame, const Face::Landmark &faceLandmark5) {
    std::vector<float> inputData = this->preProcess(visionFrame, faceLandmark5);
    std::vector<int64_t> inputImgShape = {1, 3, this->m_inputHeight, this->m_inputWidth};
    Ort::Value inputTensor = Ort::Value::CreateTensor<float>(m_inferenceSession.m_memoryInfo, inputData.data(),
                                                             inputData.size(), inputImgShape.data(),
                                                             inputImgShape.size());
    
    std::vector<Ort::Value> ortOutputs =
        m_inferenceSession.m_ortSession->Run(m_inferenceSession.m_runOptions,
                                             m_inferenceSession.m_inputNames.data(),
                                             &inputTensor, 1, m_inferenceSession.m_outputNames.data(),
                                             m_inferenceSession.m_outputNames.size());

    float *pdata = ortOutputs[0].GetTensorMutableData<float>(); /// 形状是(1, 512)
    const int lenFeature = ortOutputs[0].GetTensorTypeAndShapeInfo().GetShape()[1];

    Face::Embedding embedding(lenFeature), normedEmbedding(lenFeature);

    memcpy(embedding.data(), pdata, lenFeature * sizeof(float));

    double norm = cv::norm(embedding, cv::NORM_L2);
    for (int i = 0; i < lenFeature; i++) {
        normedEmbedding.at(i) = embedding.at(i) / (float)norm;
    }

    return {embedding, normedEmbedding};
}
