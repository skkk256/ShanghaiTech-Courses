#include "cloth_renderer.hpp"

RectClothRenderer::
RectClothRenderer(
    Shader* shader,
    FirstPersonCamera* camera,
    RectCloth* cloth,
    Mesh* mesh
) {
    this->shader = shader;
    this->camera = camera;
    this->cloth = cloth;
    this->mesh = mesh;

    this->initVertices();
    this->initIndices();
    this->updateNormals();

    this->glo.initData();
}

void RectClothRenderer::
draw() {
    this->updatePositions();
    this->updateNormals();

    // Update Data
    glBindBuffer(GL_ARRAY_BUFFER, glo.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData) * this->glo.vertices.size(), this->glo.vertices.data(), GL_STREAM_DRAW);

    GLint previous;
    glGetIntegerv(GL_POLYGON_MODE, &previous);

    this->shader->use();
    this->shader->setMat4("Projection", this->camera->getProjection());
    this->shader->setMat4("View", this->camera->getView());
    this->shader->setVec3("CameraPos", this->camera->getCameraPos());

    glBindVertexArray(glo.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, glo.VBO);

    this->shader->setBool("DrawLine", false);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FLAT); // We want flat mode
    glDrawElements(GL_TRIANGLES, (int)glo.indices.size(), GL_UNSIGNED_INT, NULL);
    this->mesh->draw();
    // this->shader->setBool("DrawLine", true);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // We want line mode
    // glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(glo.indices.size()), GL_UNSIGNED_INT, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glPolygonMode(GL_FRONT_AND_BACK, previous); // restore previous mode
}

void RectClothRenderer::
initVertices() {
    const unsigned int nh = this->cloth->nh;
    const unsigned int nw = this->cloth->nw;
    const unsigned int total = nh * nw;

    this->glo.vertices.clear();
    this->glo.vertices.resize(total);

    for (unsigned int i = 0; i < total; ++i) {
        this->glo.vertices[i].position = this->cloth->getPosition(i);
    }
}


void RectClothRenderer::
initIndices() {
    this->glo.indices.clear();

    const unsigned int nh = this->cloth->nh;
    const unsigned int nw = this->cloth->nw;

    this->glo.indices.reserve((nh - 1) * (nw - 1) * 6);
    for (unsigned int ih = 0; ih < nh - 1; ++ih) {
        for (unsigned int iw = 0; iw < nw - 1; ++iw) {
            unsigned int leftDownIdx = this->cloth->idxFromCoord(iw, ih);
            unsigned int rightDownIdx = this->cloth->idxFromCoord(iw + 1, ih);
            unsigned int leftUpIdx = this->cloth->idxFromCoord(iw, ih + 1);
            unsigned int rightUpIdx = this->cloth->idxFromCoord(iw + 1, ih + 1);

            this->glo.indices.push_back(leftDownIdx);
            this->glo.indices.push_back(rightDownIdx);
            this->glo.indices.push_back(rightUpIdx);

            this->glo.indices.push_back(leftDownIdx);
            this->glo.indices.push_back(rightUpIdx);
            this->glo.indices.push_back(leftUpIdx);
        }
    }
}

void RectClothRenderer::
updatePositions() {
    const unsigned int nh = this->cloth->nh;
    const unsigned int nw = this->cloth->nw;
    const unsigned int total = nh * nw;

    for (unsigned int i = 0; i < total; ++i) {
        this->glo.vertices[i].position = this->cloth->getPosition(i);
    }
}

void RectClothRenderer::
updateNormals() {
    const unsigned int nh = this->cloth->nh;
    const unsigned int nw = this->cloth->nw;
    const unsigned int total = nh * nw;

    for (unsigned int i = 0; i < total; ++i) {
        this->glo.vertices[i].normal = glm::vec3(0);
    }

    for (unsigned int ih = 0; ih < nh - 1; ++ih) {
        for (unsigned int iw = 0; iw < nw - 1; ++iw) {
            unsigned int leftDownIdx = this->cloth->idxFromCoord(iw, ih);
            unsigned int rightDownIdx = this->cloth->idxFromCoord(iw + 1, ih);
            unsigned int leftUpIdx = this->cloth->idxFromCoord(iw, ih + 1);
            unsigned int rightUpIdx = this->cloth->idxFromCoord(iw + 1, ih + 1);

            unsigned int lower[3] = {leftDownIdx, rightDownIdx, rightUpIdx};
            unsigned int upper[3] = {leftDownIdx, rightUpIdx, leftUpIdx};
            glm::vec3 lowerNormal = calcNormal(
                this->glo.vertices[leftDownIdx].position,
                this->glo.vertices[rightDownIdx].position,
                this->glo.vertices[rightUpIdx].position
            );
            glm::vec3 upperNormal = calcNormal(
                this->glo.vertices[leftDownIdx].position,
                this->glo.vertices[rightUpIdx].position,
                this->glo.vertices[leftUpIdx].position
            );
            for (int i = 0; i < 3; ++i) {
                this->glo.vertices[lower[i]].normal += lowerNormal;
                this->glo.vertices[upper[i]].normal += upperNormal;
            }
        }
    }

    for (unsigned int i = 0; i < total; ++i) {
        this->glo.vertices[i].normal = glm::normalize(this->glo.vertices[i].normal);
    }
}