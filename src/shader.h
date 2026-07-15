namespace shader
{
    extern unsigned int shaderId;

    void compile(const int vertexResourceId, const int fragmentResourceId);
    void cleanUp();
    void setVec2(const char* name, float x, float y);
}