/*
  MIT License

  Copyright (c) 2026 Stefan Mader

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
#include "GL.H"
#include "SERIALZE.HPP"
#include "DIALOGS.HPP"
#include "GAMEOBJ.HPP"
#include "STL/SET.HPP"
#include "STL/IMAGE.HPP"
#include "TEXTURES.HPP"
#include "RENDER.HPP"
#define CGLTF_IMPLEMENTATION
#include "cgltf.hpp"
#include "STL/SMPLOBJL.HPP"
#include "STL/SPRTEOBJ.HPP"
#include "STL/DOS.HPP"
#include "STL/PARSON.HPP"
#include "STL/JSONWRT.HPP"
#include <math.h>
#include "OUTLINER.HPP"
#include "TOOLBAR.HPP"
#include "VIEWPORT.HPP"
#include "PAINT.HPP"

int version = 100;

char *fileTypeNames[FILETYPE_COUNT] = {"none","skybox","mesh","billboard"};

enum JSONTYPE {
  JSONTYPE_NULL = 0,
  JSONTYPE_BOOL = 1,
  JSONTYPE_INT = 2,
  JSONTYPE_UINT = 3,
  JSONTYPE_FLOAT = 4,
  JSONTYPE_DOUBLE = 5,
  JSONTYPE_STRING = 6,
  JSONTYPE_NAME = 7,
  JSONTYPE_ARRAY_OPEN = 8,
  JSONTYPE_ARRAY_CLOSE = 9,
  JSONTYPE_OBJECT_OPEN = 10,
  JSONTYPE_OBJECT_CLOSE = 11,
  JSONTYPE_VECTOR = 12,
  JSONTYPE_QUATERNION = 13,
  JSONTYPE_FLOAT4 = 14,
  JSONTYPE_MATRIX = 15
};

JSON_Value * json_parse_string(const char *string);

#define JSONPORTIONLENGTH 1024

#define STARTSERIALIZATION(__save__, __file__) int arrayIndex = 0; bool error = false; JSON_Value *jval; Array<JSON_Value*> jvals; Array<char> jsonArray; jsonArray.resize(JSONPORTIONLENGTH); jsonArray[0] = '{'; char *jsonPos = &jsonArray[1]; bool jsonIsSaving = __save__; if (!jsonIsSaving) {loadArray(&jsonArray,file);jval=json_parse_string_with_comments(&jsonArray[0]); jvals.push_back(jval); jsonPos = (char*)&jvals; if (jval==NULL) error = true;}
#define FINISHSERIALIZATION(__file__) {if (jsonIsSaving) {if (jsonPos[-1] == ',' ) jsonPos--; sprintf(jsonPos,"}");fputs(&jsonArray[0],file);}}

#define S_NULL(__name__)  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_NULL, __name__, NULL, jsonIsSaving,arrayIndex)
#define S_BOOL(__name__, __v__)  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_BOOL, __name__, &(__v__), jsonIsSaving,arrayIndex)
#define S_INT(__name__, __v__)  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_INT, __name__, &(__v__), jsonIsSaving,arrayIndex)
#define S_UINT(__name__, __v__)  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_UINT, __name__, &(__v__), jsonIsSaving,arrayIndex)
#define S_FLOAT(__name__, __v__)  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_FLOAT, __name__, &(__v__), jsonIsSaving,arrayIndex)
#define S_DOUBLE(__name__, __v__)  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_DOUBLE, __name__, &(__v__), jsonIsSaving,arrayIndex)
#define S_STRING(__name__, __v__)  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_STRING, __name__, &(__v__), jsonIsSaving,arrayIndex)
#define S_NAME(__name__, __v__)  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_NAME, __name__, (__v__), jsonIsSaving,arrayIndex)
#define S_ARRAY_OPEN(__name__)  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_ARRAY_OPEN, __name__, NULL, jsonIsSaving,arrayIndex)
#define S_ARRAY_CLOSE()  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_ARRAY_CLOSE, NULL, NULL, jsonIsSaving,arrayIndex)
#define S_OBJECT_OPEN(__name__)  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_OBJECT_OPEN, __name__, NULL, jsonIsSaving,arrayIndex)
#define S_OBJECT_CLOSE()  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_OBJECT_CLOSE, NULL, NULL, jsonIsSaving,arrayIndex)
#define S_VECTOR(__name__, __v__)  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_VECTOR, __name__, &(__v__), jsonIsSaving,arrayIndex)
#define S_QUATERNION(__name__, __v__)  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_QUATERNION, __name__, &(__v__), jsonIsSaving,arrayIndex)
#define S_FLOAT4(__name__, __v__)  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_FLOAT4, __name__, &(__v__), jsonIsSaving,arrayIndex)
#define S_MATRIX(__name__, __v__)  jsonSerialize(&jsonArray, (void**)&jsonPos, JSONTYPE_MATRIX, __name__, &(__v__), jsonIsSaving,arrayIndex)
int arraySize = 0; 
#define S_ARRAY(__name__,__array__,__elementFunc__) {\
  S_ARRAY_OPEN(__name__);\
  if (!save) {\
    (__array__).clear(); (__array__).resize(arraySize);\
  }\
  int k = arrayIndex;\
  for (int i = 0; i < (__array__).size(); i++) {\
    arrayIndex = i;\
    __elementFunc__;\
  }\
  arrayIndex = k;\
  S_ARRAY_CLOSE();\
}
#define S_ARRAY_POINTER_RESIZE(__array__,__type__,__count__) {\
  if (!save) {\
    for (int i1 = 0; i1 < (__array__).size(); i1++) if ((__array__)[i1] != NULL) {delete (__array__)[i1]; (__array__)[i1]=NULL;}\
    (__array__).clear(); (__array__).resize(__count__);\
    for (int i2 = 0; i2 < (__array__).size(); i2++) (__array__)[i2] = new (__type__);\
  }\
}
#define S_CARRAY(__name__,__array__,__elementFunc__,__count__) {\
  S_ARRAY_OPEN(__name__);\
  int k = arrayIndex;\
  for (int i = 0; i < (__count__); i++) {\
    arrayIndex = i;\
    __elementFunc__;\
  }\
  arrayIndex = k;\
  S_ARRAY_CLOSE();\
}

char tempBuffer[256];

void jsonSerialize(void *json, void **jsonPos, int type, const char *name, void *data, bool save, int arrayIndex) {
  if (save) {
    Array<char> *w = (Array<char>*)json;
    char *oldPos = *(char**)jsonPos;
    char *newPos = oldPos;
    unsigned int len = &(*w)[w->size()] - oldPos;
    switch(type) {
      case JSONTYPE_NULL: {newPos = json_null( oldPos, name, &len );} break;
      case JSONTYPE_BOOL: {newPos = json_bool( oldPos, name, *((bool*)data), &len );} break;
      case JSONTYPE_INT: {newPos = json_int( oldPos, name, *((int*)data), &len );} break;
      case JSONTYPE_UINT: {newPos = json_uint( oldPos, name, *((unsigned int*)data), &len );} break;
      case JSONTYPE_FLOAT: {double v = *((float*)data); newPos = json_double( oldPos, name, v, &len );} break;
      case JSONTYPE_DOUBLE: {newPos = json_double( oldPos, name, *((double*)data), &len );} break;
      case JSONTYPE_STRING: {String str = *(String*)data; newPos = json_str( oldPos, name, str.c_str(), &len );} break;
      case JSONTYPE_NAME: {String str = (const char *)data; newPos = json_str( oldPos, name, str.c_str(), &len );} break;
      case JSONTYPE_ARRAY_OPEN: {newPos = json_arrOpen( oldPos, name, &len );} break;
      case JSONTYPE_ARRAY_CLOSE: {newPos = json_arrClose( oldPos,&len );} break;
      case JSONTYPE_OBJECT_OPEN: {newPos = json_objOpen( oldPos, name, &len );} break;
      case JSONTYPE_OBJECT_CLOSE: {newPos = json_objClose( oldPos,&len );} break;
      case JSONTYPE_VECTOR: {Vector *v = (Vector*)data; sprintf(tempBuffer,"%f,%f,%f,%f", v->x, v->y, v->z, v->w); newPos = json_str( oldPos, name, tempBuffer, &len );} break;
      case JSONTYPE_QUATERNION: {Quaternion *v = (Quaternion*)data; sprintf(tempBuffer,"%f,%f,%f,%f", v->x, v->y, v->z, v->w); newPos = json_str( oldPos, name, tempBuffer, &len );} break;
      case JSONTYPE_FLOAT4: {float *v = (float*)data; sprintf(tempBuffer,"%f,%f,%f,%f", v[0], v[1], v[2], v[3]); newPos = json_str( oldPos, name, tempBuffer, &len );} break;
      case JSONTYPE_MATRIX: {double *v = ((Matrix*)data)->m; sprintf(tempBuffer,"%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f", v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7], v[8], v[9], v[10], v[11], v[12], v[13], v[14], v[15]); newPos = json_str( oldPos, name, tempBuffer, &len );} break;
    }
    int posInString = (int)(newPos - &(*w)[0]);
    if (posInString > w->size()-JSONPORTIONLENGTH) {
      w->resize(w->size() + JSONPORTIONLENGTH*2);
      newPos = &(*w)[0] + posInString;
    }
    *jsonPos = newPos;
  }
  if (!save) {
    Array<JSON_Value*> *jvals = *(Array<JSON_Value*>**)jsonPos;
    JSON_Value *jval = jvals->back();
    JSON_Value *val = NULL;
    switch(json_value_get_type(jval)) {
      case JSONObject: {val = json_object_get_value(json_value_get_object(jval), name);} break;
      case JSONArray: {val = json_array_get_value(json_value_get_array(jval), arrayIndex);} break;
    }
    switch(type) {
      case JSONTYPE_NULL: {} break;
      case JSONTYPE_BOOL: {*((bool*)data) = json_value_get_boolean(val);} break;
      case JSONTYPE_INT: {*((int*)data) = (int)json_value_get_number(val);} break;
      case JSONTYPE_UINT: {*((unsigned int*)data) = (unsigned int)json_value_get_number(val);} break;
      case JSONTYPE_FLOAT: {*((float*)data) = (float)json_value_get_number(val);} break;
      case JSONTYPE_DOUBLE: {*((double*)data) = (double)json_value_get_number(val);} break;
      case JSONTYPE_STRING: {*(String*)data = (const char *)json_value_get_string(val);} break;
      case JSONTYPE_NAME: {sprintf((char*)data,"%s",(const char *)json_value_get_string(val));} break;
      case JSONTYPE_ARRAY_OPEN: {jvals->push_back(val);arraySize=json_array_get_count(json_value_get_array(val));} break;
      case JSONTYPE_ARRAY_CLOSE: {jvals->pop_back();} break;
      case JSONTYPE_OBJECT_OPEN: {jvals->push_back(val);} break;
      case JSONTYPE_OBJECT_CLOSE: {jvals->pop_back();} break;
      case JSONTYPE_VECTOR: {Vector *v = (Vector*)data; String t = (const char *)json_value_get_string(val); sscanf(t.c_str(),"%lf,%lf,%lf,%lf", &v->x, &v->y, &v->z, &v->w);} break;
      case JSONTYPE_QUATERNION: {Quaternion *v = (Quaternion*)data; String t = (const char *)json_value_get_string(val); sscanf(t.c_str(),"%lf,%lf,%lf,%lf", &v->x, &v->y, &v->z, &v->w);} break;
      case JSONTYPE_FLOAT4: {float *v = (float*)data; String t = (const char *)json_value_get_string(val); sscanf(t.c_str(),"%f,%f,%f,%f", &v[0], &v[1], &v[2], &v[3]);} break;
      case JSONTYPE_MATRIX: {double *v = ((Matrix*)data)->m; String t = (const char *)json_value_get_string(val); sscanf(t.c_str(),"%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5], &v[6], &v[7], &v[8], &v[9], &v[10], &v[11], &v[12], &v[13], &v[14], &v[15]);} break;
    }
  }
}

void loadArray(Array<char> *arr, FILE *in) {
  fseek(in,0,SEEK_END);
  unsigned int fileLen = ftell(in);
  fseek(in,0,SEEK_SET);
  arr->resize(fileLen);
  fread(&(*arr)[0],1,fileLen,in);
}

void serializeAll(FILE *file, bool save) {
  STARTSERIALIZATION(save, file);
  if (error) {return;}

  // ID/Header
  S_STRING("id", String("SWIFE SCENE FILE"));
  S_INT("version", version);

  // CONFIG.HPP
  S_BOOL("fullScreen",fullScreen);
  S_STRING("appFolder",appFolder);
  S_INT("maxTextureWidth",maxTextureWidth);
  S_INT("maxTextureHeight",maxTextureHeight);
  S_INT("defaultRenderTargetWidth",defaultRenderTargetWidth);
  S_INT("defaultRenderTargetHeight",defaultRenderTargetHeight);

  // GAMEOBJECT.HPP // some missing
  S_ARRAY_OPEN("grids");
  for (int i1 = 0; i1 < MAXGRIDS; i1++) {
    S_GRID(NULL,&grids[i1]);
  }
  S_ARRAY_CLOSE();
  S_ARRAY_OPEN("gameObjects");
  S_ARRAY_POINTER_RESIZE(gameObjects,GameObject,arraySize);
  for (int i2 = 0; i2 < gameObjects.size(); i2++) {
    S_GAMEOBJECT(NULL,gameObjects[i2]);
  }
  S_ARRAY_CLOSE();

  // OUTLINER.HPP
  S_UINT("outlinerLayers",outlinerLayers);

  // PAINT.HPP
  S_BOOL("justDrawBoundingBoxes",justDrawBoundingBoxes);

  // RENDER.HPP
  S_INT("currentCamera",currentCamera);
  S_UINT("clearColor", clearColor);
  S_BOOL("lightOn", lightOn);
  S_BOOL("texturedOn",texturedOn);
  S_BOOL("twoSidedOn",twoSidedOn);
  S_ARRAY_OPEN("cameras");
  for (int i3 = 0; i3 < CAMERA_COUNT; i3++) {
    S_CAMERA(NULL,&cameras[i3]);
  }
  S_ARRAY_CLOSE();

  // SERIALIZE.HPP // some missing
  S_UINT("bindingUniqueId",bindingUniqueId);

  S_ARRAY_OPEN("bindings");
  S_ARRAY_POINTER_RESIZE(bindings,FileBinding,arraySize);
  for (int i4 = 0; i4 < bindings.size(); i4++) {
    S_FILEBINDING(NULL,bindings[i4]);
    if (!save) bindings[i4]->reload();
  }
  S_ARRAY_CLOSE();

  // TOOLBAR.HPP
  S_INT("mode",mode);
  S_FLOAT("cameraSpeed",cameraSpeed);


  // VIEWPORT.HPP
  S_BOOL("toolOn", toolOn);
  S_UINT("currentUniqueGameObjectId",currentUniqueGameObjectId);

  FINISHSERIALIZATION(file);
}

FileBinding::~FileBinding() {
  free();
}

void FileBinding::load(const char *name, unsigned int type) {
  this->fileName = name;
  this->type = type;
  reload();
}

Array<unsigned int> allGenTexturesHere;
unsigned int OBJloadTexture(const String &fileName, const String &type) {
  unsigned int texture = 0;
  if (type != "map_Kd") return texture;
  String name = shortFileName(fileName);
  RGBAImage image = RGBAImage::fromFile(name.c_str());
  if (image.data == NULL) return texture;
  correctTextureSize(&image);
  glGenTextures(1, &texture);
  allGenTexturesHere.push_back(texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,image.width,image.height,0,GL_RGBA,GL_UNSIGNED_BYTE,image.data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  image.free();
  return texture;
}

unsigned int vectorToColor(const Vector &v) {
  int r = (int)floor(v.x * 255.0+0.5);
  int g = (int)floor(v.y * 255.0+0.5);
  int b = (int)floor(v.z * 255.0+0.5);
  int a = (int)floor(v.w * 255.0+0.5);
  if (r < 0) r = 0; if (r > 255) r = 255;
  if (g < 0) g = 0; if (g > 255) g = 255;
  if (b < 0) b = 0; if (b > 255) b = 255;
  if (a < 0) a = 0; if (a > 255) a = 255;
  return r|(g<<8)|(b<<16)|(a<<24);
}

bool loadMeshForBinding(const String &fileName, FileBinding *binding) {
  String a = toLower(fileName);
  if (a.endsWith(".obj")) { // WAVEFRONT OBJECT IMPORT
    SMPL_File *obj = loadObj(fileName.c_str(), true);
    if (obj == NULL) return false;
    allGenTexturesHere.clear();
    obj->loadTextures(OBJloadTexture);
    binding->textureIds = allGenTexturesHere;
    bool first = true;
    binding->boundingBoxMin.set(-1,-1,-1);
    binding->boundingBoxMax.set(1,1,1);
    for (int o = 0; o < obj->objs.size(); o++) {
      SMPL_Object *so = &obj->objs[o];
      for (int f = so->faceStart; f < so->faceEnd; f++) {
        SMPL_Face *fo = &obj->faces[f];
        unsigned int _m = fo->m;
        SMPL_Material *mat = NULL;
        if (obj->materialNames.has(_m)) {
          const String &materialName = obj->materialNames[_m];
          mat = &obj->materials[materialName];
        }
        for (int t = 0; t < 3; t++) {
          Vector *_p = &obj->vertices[fo->v[t]];
          if (first||_p->x<binding->boundingBoxMin.x) binding->boundingBoxMin.x = _p->x;
          if (first||_p->y<binding->boundingBoxMin.y) binding->boundingBoxMin.y = _p->y;
          if (first||_p->z<binding->boundingBoxMin.z) binding->boundingBoxMin.z = _p->z;
          if (first||_p->x>binding->boundingBoxMax.x) binding->boundingBoxMax.x = _p->x;
          if (first||_p->y>binding->boundingBoxMax.y) binding->boundingBoxMax.y = _p->y;
          if (first||_p->z>binding->boundingBoxMax.z) binding->boundingBoxMax.z = _p->z;
          first = false;
          Vector *_n = &obj->normals[fo->n[t]];
          Vector *_t = &obj->texCoords[fo->t[t]];
          BakedVertex k;
          k.x = _p->x;
          k.y = _p->y;
          k.z = _p->z;
          k.nx = _n->x;
          k.ny = _n->y;
          k.nz = _n->z;
          k.tx = _t->x;
          k.ty = _t->y;
          k.color = 0xffffffff;
          k.textureId = 0;
          if (mat != NULL) {
            k.color = vectorToColor(mat->diffuse);
            if (mat->mapDiffuse.used && mat->mapDiffuse.glHandle != 0) {
              k.textureId = mat->mapDiffuse.glHandle;
            }
          }
          binding->triangles.push_back(k);
        }
      }
    }
    delete obj;
    return true;
  }
  return false;
}

bool FileBinding::reload() {
  free();
  renderingUpdated = true;
  switch(type) {
  case FILETYPE_SKYBOX: {RGBAImage s = RGBAImage::fromFile(fileName.c_str());
      if (s.data == NULL) {dialogMessage(true,(String("Couldn't load file:<")+fileName+">").c_str()); return false;}
      textureIds.resize(6); 
      skyBoxFromHorizontalCrossImage(&textureIds[0], &s);
      s.free();
      return true;
    } break;
  case FILETYPE_MESH: return loadMeshForBinding(fileName,this);
  case FILETYPE_BILLBOARD: {
      billBoardIsTexture = false;
      RGBAImage s = RGBAImage::fromFile(fileName.c_str());
      if (s.data != NULL) {
        billBoardOriginalWidth = s.width;
        billBoardOriginalHeight = s.height;
        textureIds.push_back(textureFromImage(&s));
        s.free();
        billBoardIsTexture = true;
        return true;
      }
      if (loadMeshForBinding(fileName, this)) {
        billBoardOriginalWidth = defaultRenderTargetWidth;
        billBoardOriginalHeight = defaultRenderTargetHeight;
        spriteObject = new SpriteObject();
        createSpriteObjectFrameBuffer(spriteObject,billBoardOriginalWidth,billBoardOriginalHeight);
        return true;
      }
      dialogMessage(true,(String("Couldn't load file:<")+fileName+">").c_str()); 
      return false;
    } break;
  }
  return false;
}

void FileBinding::free() {
  for (int i = 0; i < textureIds.size(); i++) {
    glDeleteTextures(1,&textureIds[i]);
  }
  textureIds.clear();
  triangles.clear();
  if (spriteObject != 0) {freeSpriteObject(spriteObject); delete spriteObject; spriteObject = NULL;}
}

unsigned int bindingUniqueId = 1;
Array<FileBinding*> bindings;

FileBinding *getBindingById(unsigned int uniqueId) {
  for (int i = 0; i < bindings.size(); i++) {
    if (bindings[i]->uniqueId == uniqueId) return bindings[i];
  }
  return NULL;
}

FileBinding *getBindingByFileName(const char *fileName, unsigned int type) {
  for (int i = 0; i < bindings.size(); i++) {
    if (bindings[i]->fileName == fileName && bindings[i]->type == type) return bindings[i];
  }
  return NULL;
}

FileBinding *newBinding() {
  FileBinding *ret = new FileBinding();
  bindings.push_back(ret);
  return ret;
}

void clearAllUnusedBindings() {
  Set<unsigned int> usedBindings;
  for (int i = 0; i < gameObjects.size(); i++) {
    usedBindings.insert(gameObjects[i]->fileBindingId);
  }
  for (int j = 0; j < bindings.size(); j++) {
    if (!usedBindings.has(bindings[j]->uniqueId)) {
      delete bindings[j];
      bindings.erase(j,1);
      j--;
    }
  }
}

void freeBindings() {
  for (int i = 0; i < bindings.size(); i++) {
    delete bindings[i];
  }
  bindings.clear();
}

void loadScene(const char *path, const char *fileName) {
  String fullName = String(path) + fileName;
  FILE *in = fopen(shortFileName(fullName).c_str(),"rb");
  if (in == NULL) return;
  serializeAll(in,false);
  fclose(in);
}

void saveScene(const char *path, const char *fileName) {
  String fileName2 = fileName;
  String tempName = String(path)+"TMP.TMP";
  dosRemoveFile(tempName.c_str());
  String fullName = String(path) + fileName2;
  FILE *out = fopen(tempName.c_str(),"w");
  if (out == NULL) return;
  serializeAll(out, true);
  fclose(out);
  if (!doslfnDeleteFiles(fullName.c_str(),false)) {
    dosRemoveFile(fullName.c_str());
  }
  if (!doslfnRename(tempName.c_str(),fullName.c_str())) {
    rename(tempName.c_str(),fullName.c_str());
  }
}

void loadSkyBoxForObject(class GameObject *o, const char *fileName) {
  FileBinding *binding = getBindingByFileName(fileName, FILETYPE_SKYBOX);
  if (binding == NULL) {
    binding = newBinding();
    binding->load(fileName, FILETYPE_SKYBOX);
  }
  o->fileBindingId = binding->uniqueId;
}

void loadSkyBox(const char *path, const char *fileName) {
  int j = getObjectIdByUniqueId(dialogLoadSaveGameObjectUniqueId);
  if (j<0) return;
  GameObject *o = gameObjects[j];
  loadSkyBoxForObject(o,(shortFileName(String(path)+String(fileName))).c_str());
}

void loadMeshForObject(class GameObject *o, const char *fileName) {
  FileBinding *binding = getBindingByFileName(fileName, FILETYPE_MESH);
  if (binding == NULL) {
    binding = newBinding();
    binding->load(fileName, FILETYPE_MESH);
  }
  o->fileBindingId = binding->uniqueId;
}

void loadMesh(const char *path, const char *fileName) {
  int j = getObjectIdByUniqueId(dialogLoadSaveGameObjectUniqueId);
  if (j<0) return;
  GameObject *o = gameObjects[j];
  loadMeshForObject(o,(shortFileName(String(path)+String(fileName))).c_str());
}

void loadBillBoardForObject(class GameObject *o, const char *fileName) {
  FileBinding *binding = getBindingByFileName(fileName, FILETYPE_BILLBOARD);
  if (binding == NULL) {
    binding = newBinding();
    binding->load(fileName, FILETYPE_BILLBOARD);
  }
  o->fileBindingId = binding->uniqueId;
  if (binding->billBoardIsTexture) {
    o->billBoardTX0 = 0;
    o->billBoardTY0 = 0;
    o->billBoardTX1 = binding->billBoardOriginalWidth;
    o->billBoardTY1 = binding->billBoardOriginalHeight;
    if (binding->billBoardOriginalWidth != 0 && binding->billBoardOriginalHeight != 0) {
      o->billBoardAspect = (double)binding->billBoardOriginalWidth/binding->billBoardOriginalHeight;
    }
  }
}

void loadBillBoard(const char *path, const char *fileName) {
  int j = getObjectIdByUniqueId(dialogLoadSaveGameObjectUniqueId);
  if (j<0) return;
  GameObject *o = gameObjects[j];
  loadBillBoardForObject(o,(shortFileName(String(path)+String(fileName))).c_str());
}
