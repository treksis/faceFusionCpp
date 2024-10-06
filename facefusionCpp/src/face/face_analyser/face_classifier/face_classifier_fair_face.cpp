/**
 ******************************************************************************
 * @file           : face_classifier_fair_face.cpp
 * @author         : CodingRookie
 * @brief          : None
 * @attention      : None
 * @date           : 24-10-15
 ******************************************************************************
 */

#include "face_classifier_fair_face.h"

FaceClassifierFairFace::FaceClassifierFairFace(const std::shared_ptr<Ort::Env> &env, const std::string &modelPath) :
    FaceClassifierBase(env, modelPath) {
    m_inputWidth = m_inferenceSession.m_inputNodeDims[0][2];
    m_inputHeight = m_inferenceSession.m_inputNodeDims[0][3];
}

std::vector<float> FaceClassifierFairFace::getInputImageData(const cv::Mat &image, const Face::Landmark &faceLandmark5) {
    cv::Mat inputImage;
    std::tie(inputImage, std::ignore) = FaceHelper::warpFaceByFaceLandmarks5(image, faceLandmark5, FaceHelper::getWarpTemplate(m_WarpTemplateType), m_size);

    std::vector<cv::Mat> inputChannels(3);
    cv::split(inputImage, inputChannels);
    for (int c = 0; c < 3; c++) {
        inputChannels[c].convertTo(inputChannels[c], CV_32FC1, 1 / (255.0 * m_standardDeviation.at(c)),
                                   -m_mean.at(c) / m_standardDeviation.at(c));
    }

    const int imageArea = inputImage.cols * inputImage.rows;
    std::vector<float> inputData(imageArea * 3);
    size_t singleChannelSize = imageArea * sizeof(float);
    memcpy(inputData.data(), (float *)inputChannels[2].data, singleChannelSize);                 // R
    memcpy(inputData.data() + imageArea, (float *)inputChannels[1].data, singleChannelSize);     // G
    memcpy(inputData.data() + 2 * imageArea, (float *)inputChannels[0].data, singleChannelSize); // B
    return inputData;
}

FaceClassifierBase::Result FaceClassifierFairFace::classify(const cv::Mat &image, const Face::Landmark &faceLandmark5) {
    std::vector<Ort::Value> inputTensor;
    std::vector<int64_t> inputShape{1, 3, m_inputHeight, m_inputWidth};
    std::vector<float> inputData = getInputImageData(image, faceLandmark5);
    inputTensor.emplace_back(Ort::Value::CreateTensor<float>(m_inferenceSession.m_memoryInfo,
                                                             inputData.data(), inputData.size(),
                                                             inputShape.data(), inputShape.size()));

    std::vector<Ort::Value> outputTensor = m_inferenceSession.m_ortSession->Run(m_inferenceSession.m_runOptions,
                                                                                m_inferenceSession.m_inputNames.data(),
                                                                                inputTensor.data(), inputTensor.size(),
                                                                                m_inferenceSession.m_outputNames.data(),
                                                                                m_inferenceSession.m_outputNames.size());

    int64 raceId = outputTensor[0].GetTensorMutableData<int64>()[0];
    int64 genderId = outputTensor[1].GetTensorMutableData<int64>()[0];
    int64 ageId = outputTensor[2].GetTensorMutableData<int64>()[0];

    Result result{};
    result.age = categorizeAge(ageId);
    result.gender = categorizeGender(genderId);
    result.race = categorizeRace(raceId);
    return result;
}

Face::Age FaceClassifierFairFace::categorizeAge(const int64_t &ageId) {
    Face::Age age{};

    if (ageId == 0) {
        age.min = 0;
        age.max = 2;
    } else if (ageId == 1) {
        age.min = 3;
        age.max = 9;
    } else if (ageId == 2) {
        age.min = 10;
        age.max = 19;
    } else if (ageId == 3) {
        age.min = 20;
        age.max = 29;
    } else if (ageId == 4) {
        age.min = 30;
        age.max = 39;
    } else if (ageId == 5) {
        age.min = 40;
        age.max = 49;
    } else if (ageId == 6) {
        age.min = 50;
        age.max = 59;
    } else if (ageId == 7) {
        age.min = 60;
        age.max = 69;
    } else {
        age.min = 70;
        age.max = 100;
    }

    return age;
}

Face::Gender FaceClassifierFairFace::categorizeGender(const int64_t &genderId) {
    if (genderId == 0) {
        return Face::Male;
    } else {
        return Face::Female;
    }
}

Face::Race FaceClassifierFairFace::categorizeRace(const int64_t &raceId) {
    if (raceId == 1) {
        return Face::Black;
    } else if (raceId == 2) {
        return Face::Latino;
    } else if (raceId == 3 || raceId == 4) {
        return Face::Asian;
    } else if (raceId == 5) {
        return Face::Indian;
    } else if (raceId == 6) {
        return Face::Arabic;
    } else {
        return Face::White;
    }
}
