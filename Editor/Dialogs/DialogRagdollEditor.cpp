#include "DialogRagdollEditor.h"

#include <map>

#include "imgui.h"
#include "imgui_internal.h"
#include "misc/cpp/imgui_stdlib.h"

#include "../Engine/Components/Transform.h"
#include "../Engine/Components/RigidBody.h"
#include "../Engine/Components/BoxCollider.h"
#include "../Engine/Components/CapsuleCollider.h"
#include "../Engine/Components/SphereCollider.h"
#include "../Engine/Components/FreeJoint.h"
#include "../Engine/Components/ConeTwistJoint.h"
#include "../Engine/Math/Mathf.h"
#include "../Engine/Core/Engine.h"
#include "../Engine/Core/GameObject.h"

#include "../Windows/MainWindow.h"
#include "../Windows/ConsoleWindow.h"
#include "../Classes/TreeNode.h"

namespace GX
{
    DialogRagdollEditor::DialogRagdollEditor()
    {
    }

    DialogRagdollEditor::~DialogRagdollEditor()
    {
    }

    void DialogRagdollEditor::show()
    {
        visible = true;

        pelvisObject = nullptr;
        lhipObject = nullptr;
        lkneeObject = nullptr;
        rhipObject = nullptr;
        rkneeObject = nullptr;
        larmObject = nullptr;
        lelbowObject = nullptr;
        rarmObject = nullptr;
        relbowObject = nullptr;
        mspineObject = nullptr;
        headObject = nullptr;
    }

    void DialogRagdollEditor::update()
    {
        if (!visible)
            return;

        ImGui::SetNextWindowSize(ImVec2(300.0f, 370.0f));
        ImGui::SetNextWindowPos(ImVec2(MainWindow::getSingleton()->getWidth() / 2, MainWindow::getSingleton()->getHeight() / 2), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
        ImGui::Begin("Create Ragdoll", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking);

        std::string pelvisText = "None";
        std::string lhipText = "None";
        std::string lkneeText = "None";
        std::string rhipText = "None";
        std::string rkneeText = "None";
        std::string larmText = "None";
        std::string lelbowText = "None";
        std::string rarmText = "None";
        std::string relbowText = "None";
        std::string mspineText = "None";
        std::string headText = "None";

        if (pelvisObject != nullptr) pelvisText = pelvisObject->getGameObject()->getName();
        if (lhipObject != nullptr) lhipText = lhipObject->getGameObject()->getName();
        if (lkneeObject != nullptr) lkneeText = lkneeObject->getGameObject()->getName();
        if (rhipObject != nullptr) rhipText = rhipObject->getGameObject()->getName();
        if (rkneeObject != nullptr) rkneeText = rkneeObject->getGameObject()->getName();
        if (larmObject != nullptr) larmText = larmObject->getGameObject()->getName();
        if (lelbowObject != nullptr) lelbowText = lelbowObject->getGameObject()->getName();
        if (rarmObject != nullptr) rarmText = rarmObject->getGameObject()->getName();
        if (relbowObject != nullptr) relbowText = relbowObject->getGameObject()->getName();
        if (mspineObject != nullptr) mspineText = mspineObject->getGameObject()->getName();
        if (headObject != nullptr) headText = headObject->getGameObject()->getName();

        ImGui::BeginColumns(std::string("Pelvis_columns").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
        ImGui::Text("Pelvis"); ImGui::NextColumn();
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##1", &pelvisText, ImGuiInputTextFlags_ReadOnly);
        acceptDragDrop(pelvisObject);
        ImGui::EndColumns();

        ImGui::BeginColumns(std::string("LeftHip_columns").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
        ImGui::Text("Left Hip"); ImGui::NextColumn();
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##2", &lhipText, ImGuiInputTextFlags_ReadOnly);
        acceptDragDrop(lhipObject);
        ImGui::EndColumns();

        ImGui::BeginColumns(std::string("LeftKnee_columns").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
        ImGui::Text("Left Knee"); ImGui::NextColumn();
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##3", &lkneeText, ImGuiInputTextFlags_ReadOnly);
        acceptDragDrop(lkneeObject);
        ImGui::EndColumns();

        ImGui::BeginColumns(std::string("RightHip_columns").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
        ImGui::Text("Right Hip"); ImGui::NextColumn();
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##4", &rhipText, ImGuiInputTextFlags_ReadOnly);
        acceptDragDrop(rhipObject);
        ImGui::EndColumns();

        ImGui::BeginColumns(std::string("RightKnee_columns").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
        ImGui::Text("Right Knee"); ImGui::NextColumn();
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##5", &rkneeText, ImGuiInputTextFlags_ReadOnly);
        acceptDragDrop(rkneeObject);
        ImGui::EndColumns();

        ImGui::BeginColumns(std::string("LeftArm_columns").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
        ImGui::Text("Left Arm"); ImGui::NextColumn();
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##6", &larmText, ImGuiInputTextFlags_ReadOnly);
        acceptDragDrop(larmObject);
        ImGui::EndColumns();

        ImGui::BeginColumns(std::string("LeftElbow_columns").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
        ImGui::Text("Left Elbow"); ImGui::NextColumn();
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##7", &lelbowText, ImGuiInputTextFlags_ReadOnly);
        acceptDragDrop(lelbowObject);
        ImGui::EndColumns();

        ImGui::BeginColumns(std::string("RightArm_columns").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
        ImGui::Text("Right Arm"); ImGui::NextColumn();
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##8", &rarmText, ImGuiInputTextFlags_ReadOnly);
        acceptDragDrop(rarmObject);
        ImGui::EndColumns();

        ImGui::BeginColumns(std::string("RightElbow_columns").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
        ImGui::Text("Right Elbow"); ImGui::NextColumn();
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##9", &relbowText, ImGuiInputTextFlags_ReadOnly);
        acceptDragDrop(relbowObject);
        ImGui::EndColumns();

        ImGui::BeginColumns(std::string("MiddleSpine_columns").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
        ImGui::Text("Middle Spine"); ImGui::NextColumn();
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##10", &mspineText, ImGuiInputTextFlags_ReadOnly);
        acceptDragDrop(mspineObject);
        ImGui::EndColumns();

        ImGui::BeginColumns(std::string("Head_columns").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
        ImGui::Text("Head"); ImGui::NextColumn();
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##11", &headText, ImGuiInputTextFlags_ReadOnly);
        acceptDragDrop(headObject);
        ImGui::EndColumns();

        ImGui::BeginColumns(std::string("TotalMass_columns").c_str(), 2, ImGuiColumnsFlags_NoResize | ImGuiColumnsFlags_NoBorder);
        ImGui::Text("Total Mass"); ImGui::NextColumn();
        ImGui::SetNextItemWidth(-1);
        ImGui::InputFloat("##12", &massValue);
        ImGui::EndColumns();

        std::string errorString = checkConsistency();
        calculateAxes();
        std::string helpString = "";

        if (errorString.length() != 0)
        {
            helpString = "Drag all bones from the hierarchy into their slots.\nMake sure your character is in T-Stand.\n";
        }
        else
        {
            helpString = "Make sure your character is in T-Stand.\nMake sure the blue axis faces in the same\ndirection the character is looking.";
        }

        ImGui::TextColored(ImVec4(1.0f / 255.0f * 158.0f, 1.0f / 255.0f * 195.0f, 1.0f, 1.0f), helpString.c_str());

        if (!errorString.empty())
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), errorString.c_str());

        bool isValid = errorString.length() == 0;

        bool closed = false;
        bool ok = false;

        ImGui::Separator();

        if (ImGui::Button("Clear"))
        {
            pelvisObject = nullptr;
            lhipObject = nullptr;
            lkneeObject = nullptr;
            rhipObject = nullptr;
            rkneeObject = nullptr;
            larmObject = nullptr;
            lelbowObject = nullptr;
            rarmObject = nullptr;
            relbowObject = nullptr;
            mspineObject = nullptr;
            headObject = nullptr;
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel"))
        {
            closed = true;
            ok = false;
        }

        if (isValid)
        {
            ImGui::SameLine();

            if (ImGui::Button("Create"))
            {
                closed = true;
                ok = true;
            }
        }

        ImGui::End();

        if (closed && ok)
        {
            createRagdoll();
        }

        if (closed)
            visible = false;
    }

    void DialogRagdollEditor::acceptDragDrop(Transform *& sceneNode)
    {
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_TreeView"))
            {
                TreeNode* move_from = (TreeNode*)payload->Data;
                GameObject* obj = Engine::getSingleton()->getGameObject(move_from->name);
                if (obj != nullptr)
                    sceneNode = obj->getTransform();
            }
            ImGui::EndDragDropTarget();
        }
    }

    void DialogRagdollEditor::createRagdoll()
    {
        cleanup(pelvisObject);

        buildCapsules();
        addBreastColliders();
        addHeadCollider();

        buildBodies();
        buildJoints();
        calculateMass();
    }

    std::string DialogRagdollEditor::checkConsistency()
    {
        prepareBones();
        std::map<Transform*, BoneInfoStruct*> map;

        for(BoneInfoStruct * bone : bones)
        {
            if (bone->anchor)
            {
                if (map[bone->anchor] != nullptr)
                {
                    BoneInfoStruct * oldBone = (BoneInfoStruct*)map[bone->anchor];
                    return bone->name + " and " + oldBone->name + "\nmay not be assigned to the same bone.";
                }
                map[bone->anchor] = bone;
            }
        }

        for(BoneInfoStruct * bone : bones)
        {
            if (bone->anchor == nullptr)
                return bone->name + " has not been assigned yet.\n";
        }

        return "";
    }

    void DialogRagdollEditor::decomposeVector(glm::vec3 & normalCompo, glm::vec3 & tangentCompo, glm::vec3 outwardDir, glm::vec3 outwardNormal)
    {
        outwardNormal = glm::normalize(outwardNormal);
        normalCompo = outwardNormal * glm::dot(outwardDir, outwardNormal);
        tangentCompo = outwardDir - normalCompo;
    }

    void DialogRagdollEditor::calculateAxes()
    {
        if (headObject != nullptr && pelvisObject != nullptr)
            up = calculateDirectionAxis(Mathf::inverseTransformPoint(pelvisObject, headObject->getPosition()));

        if (relbowObject != nullptr && pelvisObject != nullptr)
        {
            glm::vec3 removed, temp;
            decomposeVector(temp, removed, Mathf::inverseTransformPoint(pelvisObject, relbowObject->getPosition()), up);
            right = calculateDirectionAxis(removed);
        }

        forward = glm::cross(right, up);
        if (flipForward)
            forward = -forward;
    }

    void DialogRagdollEditor::prepareBones()
    {
        if (pelvisObject)
        {
            worldRight = Mathf::transformDirection(pelvisObject, right);
            worldUp = Mathf::transformDirection(pelvisObject, up);
            worldForward = Mathf::transformDirection(pelvisObject, forward);
        }

        bones = std::vector<BoneInfoStruct*>();

        rootBone = new BoneInfoStruct();
        rootBone->name = "Pelvis";
        rootBone->anchor = pelvisObject;
        rootBone->parent = nullptr;
        rootBone->density = 2.5F;
        bones.push_back(rootBone);

        glm::vec3 minLimit = glm::vec3(20, 20, 20) * Mathf::fDeg2Rad;
        glm::vec3 maxLimit = glm::vec3(0, 0, 0) * Mathf::fDeg2Rad;

        addMirroredJoint("Hips", lhipObject, rhipObject, "Pelvis", worldRight, worldForward, minLimit, maxLimit, JointType::ConeTwist, "CapsuleCollider", 0.18F, 1.5F);

        minLimit = glm::vec3(0, 0, -110) * Mathf::fDeg2Rad;
        maxLimit = glm::vec3(0, 0, 0) * Mathf::fDeg2Rad;

        addMirroredJoint("Knee", lkneeObject, rkneeObject, "Hips", worldRight, worldForward, minLimit, maxLimit, JointType::Free, "CapsuleCollider", 0.16F, 1.5F);

        minLimit = glm::vec3(20, 20, 20) * Mathf::fDeg2Rad;
        maxLimit = glm::vec3(0, 0, 0) * Mathf::fDeg2Rad;

        addJoint("Middle Spine", mspineObject, "Pelvis", worldRight, worldForward, minLimit, maxLimit, JointType::ConeTwist, "", 0.40f, 2.5F);

        minLimit = glm::vec3(70, 70, 20) * Mathf::fDeg2Rad;
        maxLimit = glm::vec3(0, 0, 0) * Mathf::fDeg2Rad;

        addMirroredJoint("Arm", larmObject, rarmObject, "Middle Spine", worldForward, worldRight, minLimit, maxLimit, JointType::ConeTwist, "CapsuleCollider", 0.30F, 1.0F);

        minLimit = glm::vec3(0, 0, -110) * Mathf::fDeg2Rad;
        maxLimit = glm::vec3(0, 0, 0) * Mathf::fDeg2Rad;

        addMirroredJoint("Elbow", lelbowObject, relbowObject, "Arm", worldRight, worldForward, minLimit, maxLimit, JointType::Free, "CapsuleCollider", 0.105F, 1.0F);

        minLimit = glm::vec3(30, 30, 30) * Mathf::fDeg2Rad;
        maxLimit = glm::vec3(0, 0, 0) * Mathf::fDeg2Rad;

        addJoint("Head", headObject, "Middle Spine", worldRight, worldForward, minLimit, maxLimit, JointType::ConeTwist, "", 1, 1.0F);
    }

    BoneInfoStruct * DialogRagdollEditor::findBone(std::string name)
    {
        for (BoneInfoStruct * bone : bones)
        {
            if (bone->name == name)
                return bone;
        }
        return nullptr;
    }

    void DialogRagdollEditor::addMirroredJoint(std::string name, Transform* leftAnchor, Transform* rightAnchor, std::string parent, glm::vec3 worldTwistAxis, glm::vec3 worldSwingAxis, glm::vec3 minLimit, glm::vec3 maxLimit, JointType jointType, std::string colliderType, float radiusScale, float density)
    {
        addJoint("Left " + name, leftAnchor, parent, worldTwistAxis, worldSwingAxis, minLimit, maxLimit, jointType, colliderType, radiusScale, density);
        addJoint("Right " + name, rightAnchor, parent, worldTwistAxis, worldSwingAxis, minLimit, maxLimit, jointType, colliderType, radiusScale, density);
    }

    void DialogRagdollEditor::addJoint(std::string name, Transform* anchor, std::string parent, glm::vec3 worldTwistAxis, glm::vec3 worldSwingAxis, glm::vec3 minLimit, glm::vec3 maxLimit, JointType jointType, std::string colliderType, float radiusScale, float density)
    {
        BoneInfoStruct * bone = new BoneInfoStruct();
        bone->name = name;
        bone->anchor = anchor;
        bone->axis = worldTwistAxis;
        bone->normalAxis = worldSwingAxis;
        bone->minLimit = minLimit;
        bone->maxLimit = maxLimit;
        bone->jointType = jointType;
        bone->density = density;
        bone->colliderType = colliderType;
        bone->radiusScale = radiusScale;

        if (findBone(parent) != nullptr)
            bone->parent = findBone(parent);
        else if (name.rfind("Left", 0) != std::string::npos)
            bone->parent = findBone("Left " + parent);
        else if (name.rfind("Right", 0) != std::string::npos)
            bone->parent = findBone("Right " + parent);

        bone->parent->children.push_back(bone);
        bones.push_back(bone);
    }

    void GetChildren(Transform * root, std::vector<Transform*> & outList)
    {
        for (auto it = root->getChildren().begin(); it != root->getChildren().end(); ++it)
        {
            Transform* child = *it;
            outList.push_back(child);
            GetChildren(child, outList);
        }
    }

    void DialogRagdollEditor::buildCapsules()
    {
        for (BoneInfoStruct * bone : bones)
        {
            if (bone->colliderType != "CapsuleCollider")
                continue;

            int direction = 0;
            float distance = 0;

            if (bone->children.size() == 1)
            {
                BoneInfoStruct * childBone = bone->children[0];
                glm::vec3 endPoint = Mathf::inverseTransformPoint(bone->anchor, childBone->anchor->getPosition());
                calculateDirection(endPoint, direction, distance);
            }
            else
            {
                glm::vec3 endPoint = Mathf::inverseTransformPoint(bone->anchor, (bone->anchor->getPosition() - bone->parent->anchor->getPosition()) + bone->anchor->getPosition());
                calculateDirection(endPoint, direction, distance);

                //Bounds bounds;
                AxisAlignedBox bounds;
                std::vector<Transform*> transforms;
                GetChildren(bone->anchor, transforms);

                for (Transform* child : transforms)
                {
                    bounds.merge(Mathf::inverseTransformPoint(bone->anchor, child->getPosition()));
                }

                if (distance > 0)
                    distance = bounds.getMaximum()[direction];
                else
                    distance = bounds.getMinimum()[direction];

            }

            CapsuleCollider* collider = new CapsuleCollider();
            glm::vec3 center = glm::vec3(0, 0, 0);

            if (direction == 0)
                collider->setRotation(Mathf::toQuaternion(glm::vec3(0, 0, 90)));

            if (direction == 1)
                collider->setRotation(Mathf::toQuaternion(glm::vec3(0, 90, 0)));

            if (direction == 2)
                collider->setRotation(Mathf::toQuaternion(glm::vec3(90, 0, 0)));

            center[direction] = distance * 0.5f;

            float radius = abs(distance * bone->radiusScale);
            float height = abs(distance);

            collider->setOffset(center);
            collider->setHeight(height - (radius * 2.0f));
            collider->setRadius(radius);

            bone->direction = direction;

            bone->anchor->getGameObject()->addComponent(collider);
        }
    }

    void DialogRagdollEditor::cleanup(Transform* root)
    {
        GameObject* obj = root->getGameObject();

        FreeJoint* joint = (FreeJoint*)obj->getComponent(FreeJoint::COMPONENT_TYPE);
        while (joint != nullptr)
        {
            obj->removeComponent(joint);
            joint = (FreeJoint*)obj->getComponent(FreeJoint::COMPONENT_TYPE);
        }

        ConeTwistJoint* joint2 = (ConeTwistJoint*)obj->getComponent(ConeTwistJoint::COMPONENT_TYPE);
        while (joint2 != nullptr)
        {
            obj->removeComponent(joint2);
            joint2 = (ConeTwistJoint*)obj->getComponent(ConeTwistJoint::COMPONENT_TYPE);
        }

        RigidBody* body = (RigidBody*)obj->getComponent(RigidBody::COMPONENT_TYPE);
        while (body != nullptr)
        {
            obj->removeComponent(body);
            body = (RigidBody*)obj->getComponent(RigidBody::COMPONENT_TYPE);
        }

        CapsuleCollider* capsule = (CapsuleCollider*)root->getGameObject()->getComponent(CapsuleCollider::COMPONENT_TYPE);
        while (capsule != nullptr)
        {
            obj->removeComponent(capsule);
            capsule = (CapsuleCollider*)obj->getComponent(CapsuleCollider::COMPONENT_TYPE);
        }

        SphereCollider* sphere = (SphereCollider*)obj->getComponent(SphereCollider::COMPONENT_TYPE);
        while (sphere != nullptr)
        {
            obj->removeComponent(sphere);
            sphere = (SphereCollider*)obj->getComponent(SphereCollider::COMPONENT_TYPE);
        }

        BoxCollider* box = (BoxCollider*)obj->getComponent(BoxCollider::COMPONENT_TYPE);
        while (box != nullptr)
        {
            obj->removeComponent(box);
            box = (BoxCollider*)obj->getComponent(BoxCollider::COMPONENT_TYPE);
        }

        for (auto it = root->getChildren().begin(); it != root->getChildren().end(); ++it)
        {
            Transform* node = *it;

            cleanup(node);
        }
    }

    void DialogRagdollEditor::buildBodies()
    {
        for (BoneInfoStruct * bone : bones)
        {
            RigidBody* body = new RigidBody();
            body->setMass(bone->density);
            bone->anchor->getGameObject()->addComponent(body);
        }
    }

    void DialogRagdollEditor::buildJoints()
    {
        for (BoneInfoStruct * bone : bones)
        {
            if (bone->parent == nullptr)
                continue;

            if (bone->jointType == JointType::Free)
            {
                FreeJoint* joint = new FreeJoint();
                bone->anchor->getGameObject()->addComponent(joint);
                bone->joint = joint;

                // Setup connection and axis
                //joint->setNormalAxis(calculateDirectionAxis(Mathf::inverseTransformDirection(bone->anchor, bone->axis)));
                //joint->setSwingAxis(calculateDirectionAxis(Mathf::inverseTransformDirection(bone->anchor, bone->normalAxis)));
                //joint->setSwingAxis(Mathf::toEuler(pelvisObject->getRotation()));

                joint->setAnchor(glm::vec3(0, 0, 0));

                if (bone->parent != nullptr)
                {
                    glm::highp_quat r = glm::identity<glm::highp_quat>();
                    glm::vec3 p = Mathf::inverseTransformPoint(bone->parent->anchor, bone->anchor->getPosition());
                    joint->setConnectedAnchor(p);
                    joint->setConnectedObjectGuid(bone->parent->anchor->getGameObject()->getGuid());
                }

                joint->setLimitMin(bone->minLimit);
                joint->setLimitMax(bone->maxLimit);
                joint->setLinkedBodiesCollision(false);
            }
            else
            {
                ConeTwistJoint* joint = new ConeTwistJoint();
                bone->anchor->getGameObject()->addComponent(joint);
                bone->joint = joint;

                joint->setAnchor(glm::vec3(0, 0, 0));

                if (bone->parent != nullptr)
                {
                    glm::highp_quat r = glm::identity<glm::highp_quat>();
                    glm::vec3 p = Mathf::inverseTransformPoint(bone->parent->anchor, bone->anchor->getPosition());
                    joint->setConnectedAnchor(p);
                    joint->setConnectedObjectGuid(bone->parent->anchor->getGameObject()->getGuid());
                }

                joint->setLimits(bone->minLimit);
                joint->setLinkedBodiesCollision(false);
            }
        }
    }

    void DialogRagdollEditor::calculateMassRecurse(BoneInfoStruct * bone)
    {
        float mass = ((RigidBody*)bone->anchor->getGameObject()->getComponent(RigidBody::COMPONENT_TYPE))->getMass();
        for (BoneInfoStruct * child : bone->children)
        {
            calculateMassRecurse(child);
            mass += child->summedMass;
        }
        bone->summedMass = mass;
    }

    void DialogRagdollEditor::calculateMass()
    {
        // Calculate allChildMass by summing all bodies
        calculateMassRecurse(rootBone);

        // Rescale the mass so that the whole character weights totalMass
        float massScale = massValue / rootBone->summedMass;
        for (BoneInfoStruct* bone : bones)
        {
            RigidBody* b = ((RigidBody*)bone->anchor->getGameObject()->getComponent(RigidBody::COMPONENT_TYPE));
            b->setMass(b->getMass() * massScale);
        }

        // Recalculate allChildMass by summing all bodies
        calculateMassRecurse(rootBone);
    }

    void DialogRagdollEditor::calculateDirection(glm::vec3 point, int & direction, float & distance)
    {
        // Calculate longest axis
        direction = 0;
        if (abs(point[1]) > abs(point[0]))
            direction = 1;
        if (abs(point[2]) > abs(point[direction]))
            direction = 2;

        distance = point[direction];
    }

    glm::vec3 DialogRagdollEditor::calculateDirectionAxis(glm::vec3 point)
    {
        int direction = 0;
        float distance;
        calculateDirection(point, direction, distance);
        glm::vec3 axis = glm::vec3(0, 0, 0);
        if (distance > 0)
            axis[direction] = 1.0F;
        else
            axis[direction] = -1.0F;
        return axis;
    }

    int DialogRagdollEditor::smallestComponent(glm::vec3 point)
    {
        int direction = 0;
        if (abs(point[1]) < abs(point[0]))
            direction = 1;
        if (abs(point[2]) < abs(point[direction]))
            direction = 2;
        return direction;
    }

    int DialogRagdollEditor::largestComponent(glm::vec3 point)
    {
        int direction = 0;
        if (abs(point[1]) > abs(point[0]))
            direction = 1;
        if (abs(point[2]) > abs(point[direction]))
            direction = 2;
        return direction;
    }

    int DialogRagdollEditor::secondLargestComponent(glm::vec3 point)
    {
        int smallest = smallestComponent(point);
        int largest = largestComponent(point);
        if (smallest < largest)
        {
            int temp = largest;
            largest = smallest;
            smallest = temp;
        }

        if (smallest == 0 && largest == 1)
            return 2;
        else if (smallest == 0 && largest == 2)
            return 1;
        else
            return 0;
    }

    AxisAlignedBox DialogRagdollEditor::clip(AxisAlignedBox bounds, Transform * relativeTo, Transform* clipTransform, bool below)
    {
        int axis = largestComponent(bounds.getSize());
        glm::vec3 v1 = Mathf::transformPoint(relativeTo, bounds.getMaximum());
        glm::vec3 v2 = Mathf::transformPoint(relativeTo, bounds.getMinimum());

        if (glm::dot(worldUp, v1) > glm::dot(worldUp, v2) == below)
        {
            glm::vec3 min = bounds.getMinimum();
            min[axis] = Mathf::inverseTransformPoint(relativeTo, clipTransform->getPosition())[axis];
            
            bounds.setMinimum(min);
        }
        else
        {
            glm::vec3 max = bounds.getMaximum();
            max[axis] = Mathf::inverseTransformPoint(relativeTo, clipTransform->getPosition())[axis];
            
            bounds.setMaximum(max);
        }

        return bounds;
    }

    AxisAlignedBox DialogRagdollEditor::getBreastBounds(Transform* relativeTo)
    {
        // Pelvis bounds
        AxisAlignedBox bounds;
        
        bounds.merge(Mathf::inverseTransformPoint(relativeTo, lhipObject->getPosition()));
        bounds.merge(Mathf::inverseTransformPoint(relativeTo, rhipObject->getPosition()));
        bounds.merge(Mathf::inverseTransformPoint(relativeTo, larmObject->getPosition()));
        bounds.merge(Mathf::inverseTransformPoint(relativeTo, rarmObject->getPosition()));

        glm::vec3 size = bounds.getSize();
        size[smallestComponent(bounds.getSize())] = size[largestComponent(bounds.getSize())] / 2.0F;
        bounds.setExtents(bounds.getCenter() - size * 0.5f, bounds.getCenter() + size * 0.5f);

        return bounds;
    }

    void DialogRagdollEditor::addBreastColliders()
    {
        // Middle spine and pelvis
        if (mspineObject != nullptr && pelvisObject != nullptr)
        {
            AxisAlignedBox bounds;
            BoxCollider * box = nullptr;

            // Middle spine bounds
            bounds = clip(getBreastBounds(pelvisObject), pelvisObject, mspineObject, false);

            box = new BoxCollider();
            box->setOffset(bounds.getCenter());
            box->setBoxSize(glm::abs(bounds.getSize() * 0.4f));
            pelvisObject->getGameObject()->addComponent(box);

            bounds = AxisAlignedBox();
            bounds = clip(getBreastBounds(mspineObject), mspineObject, mspineObject, true);

            box = new BoxCollider();
            box->setOffset(bounds.getCenter());
            box->setBoxSize(glm::abs(bounds.getSize() * 0.4f));
            mspineObject->getGameObject()->addComponent(box);
        }
        // Only pelvis
        else
        {
            AxisAlignedBox bounds;

            bounds.merge(Mathf::inverseTransformPoint(pelvisObject, lhipObject->getPosition()));
            bounds.merge(Mathf::inverseTransformPoint(pelvisObject, rhipObject->getPosition()));
            bounds.merge(Mathf::inverseTransformPoint(pelvisObject, larmObject->getPosition()));
            bounds.merge(Mathf::inverseTransformPoint(pelvisObject, rarmObject->getPosition()));

            glm::vec3 size = bounds.getSize();
            size[smallestComponent(bounds.getSize())] = size[largestComponent(bounds.getSize())] / 2.0F;

            BoxCollider * box = new BoxCollider();
            pelvisObject->getGameObject()->addComponent(box);
            box->setOffset(bounds.getCenter());
            box->setBoxSize(glm::abs(size * 0.4f));
        }
    }

    void DialogRagdollEditor::addHeadCollider()
    {
        if (headObject->getGameObject()->getComponent(SphereCollider::COMPONENT_TYPE))
            headObject->getGameObject()->removeComponent(headObject->getGameObject()->getComponent(SphereCollider::COMPONENT_TYPE));

        float radius = glm::distance(larmObject->getPosition(), rarmObject->getPosition());
        radius /= 4.0f;

        SphereCollider * sphere = new SphereCollider();
        headObject->getGameObject()->addComponent(sphere);
        sphere->setRadius(radius);
        glm::vec3 center = glm::vec3(0, 0, 0);

        int direction;
        float distance;

        calculateDirection(Mathf::inverseTransformPoint(headObject, pelvisObject->getPosition()), direction, distance);

        if (distance > 0)
            center[direction] = -radius;
        else
            center[direction] = radius;

        sphere->setOffset(center);
    }
}