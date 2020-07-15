// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*!
 * @file MPIMessage.cpp
 * This source file contains the definition of the described types in the IDL file.
 *
 * This file was generated by the tool gen.
 */

#ifdef _WIN32
// Remove linker warning LNK4221 on Visual Studio
namespace { char dummy; }
#endif

#include "MPIMessage.h"
#include <fastcdr/Cdr.h>

#include <fastcdr/exceptions/BadParamException.h>
using namespace eprosima::fastcdr::exception;

#include <utility>


MPIMessage::Message::Message()
{
    // m_buffer com.eprosima.idl.parser.typecode.AliasTypeCode@282003e1

    // m_senderRank com.eprosima.idl.parser.typecode.PrimitiveTypeCode@7fad8c79
    m_senderRank = 0;
    // m_recipientRank com.eprosima.idl.parser.typecode.PrimitiveTypeCode@71a794e5
    m_recipientRank = 0;
    // m_tag com.eprosima.idl.parser.typecode.PrimitiveTypeCode@76329302
    m_tag = 0;

}

MPIMessage::Message::~Message()
{




}

MPIMessage::Message::Message(const Message &x)
{
    m_buffer = x.m_buffer;
    m_senderRank = x.m_senderRank;
    m_recipientRank = x.m_recipientRank;
    m_tag = x.m_tag;
}

MPIMessage::Message::Message(Message &&x)
{
    m_buffer = std::move(x.m_buffer);
    m_senderRank = x.m_senderRank;
    m_recipientRank = x.m_recipientRank;
    m_tag = x.m_tag;
}

MPIMessage::Message& MPIMessage::Message::operator=(const Message &x)
{

    m_buffer = x.m_buffer;
    m_senderRank = x.m_senderRank;
    m_recipientRank = x.m_recipientRank;
    m_tag = x.m_tag;

    return *this;
}

MPIMessage::Message& MPIMessage::Message::operator=(Message &&x)
{

    m_buffer = std::move(x.m_buffer);
    m_senderRank = x.m_senderRank;
    m_recipientRank = x.m_recipientRank;
    m_tag = x.m_tag;

    return *this;
}

size_t MPIMessage::Message::getMaxCdrSerializedSize(size_t current_alignment)
{
    size_t initial_alignment = current_alignment;


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

    current_alignment += (100 * 1) + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);



    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);



    return current_alignment - initial_alignment;
}

size_t MPIMessage::Message::getCdrSerializedSize(const MPIMessage::Message& data, size_t current_alignment)
{
    (void)data;
    size_t initial_alignment = current_alignment;


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);

    if (data.buffer().size() > 0)
    {
        current_alignment += (data.buffer().size() * 1) + eprosima::fastcdr::Cdr::alignment(current_alignment, 1);
    }



    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);


    current_alignment += 4 + eprosima::fastcdr::Cdr::alignment(current_alignment, 4);



    return current_alignment - initial_alignment;
}

void MPIMessage::Message::serialize(eprosima::fastcdr::Cdr &scdr) const
{

    scdr << m_buffer;
    scdr << m_senderRank;
    scdr << m_recipientRank;
    scdr << m_tag;
}

void MPIMessage::Message::deserialize(eprosima::fastcdr::Cdr &dcdr)
{

    dcdr >> m_buffer;
    dcdr >> m_senderRank;
    dcdr >> m_recipientRank;
    dcdr >> m_tag;
}

/*!
 * @brief This function copies the value in member buffer
 * @param _buffer New value to be copied in member buffer
 */
void MPIMessage::Message::buffer(const MPIMessage::OctetSequence &_buffer)
{
m_buffer = _buffer;
}

/*!
 * @brief This function moves the value in member buffer
 * @param _buffer New value to be moved in member buffer
 */
void MPIMessage::Message::buffer(MPIMessage::OctetSequence &&_buffer)
{
m_buffer = std::move(_buffer);
}

/*!
 * @brief This function returns a constant reference to member buffer
 * @return Constant reference to member buffer
 */
const MPIMessage::OctetSequence& MPIMessage::Message::buffer() const
{
    return m_buffer;
}

/*!
 * @brief This function returns a reference to member buffer
 * @return Reference to member buffer
 */
MPIMessage::OctetSequence& MPIMessage::Message::buffer()
{
    return m_buffer;
}
/*!
 * @brief This function sets a value in member senderRank
 * @param _senderRank New value for member senderRank
 */
void MPIMessage::Message::senderRank(int32_t _senderRank)
{
m_senderRank = _senderRank;
}

/*!
 * @brief This function returns the value of member senderRank
 * @return Value of member senderRank
 */
int32_t MPIMessage::Message::senderRank() const
{
    return m_senderRank;
}

/*!
 * @brief This function returns a reference to member senderRank
 * @return Reference to member senderRank
 */
int32_t& MPIMessage::Message::senderRank()
{
    return m_senderRank;
}

/*!
 * @brief This function sets a value in member recipientRank
 * @param _recipientRank New value for member recipientRank
 */
void MPIMessage::Message::recipientRank(int32_t _recipientRank)
{
m_recipientRank = _recipientRank;
}

/*!
 * @brief This function returns the value of member recipientRank
 * @return Value of member recipientRank
 */
int32_t MPIMessage::Message::recipientRank() const
{
    return m_recipientRank;
}

/*!
 * @brief This function returns a reference to member recipientRank
 * @return Reference to member recipientRank
 */
int32_t& MPIMessage::Message::recipientRank()
{
    return m_recipientRank;
}

/*!
 * @brief This function sets a value in member tag
 * @param _tag New value for member tag
 */
void MPIMessage::Message::tag(int32_t _tag)
{
m_tag = _tag;
}

/*!
 * @brief This function returns the value of member tag
 * @return Value of member tag
 */
int32_t MPIMessage::Message::tag() const
{
    return m_tag;
}

/*!
 * @brief This function returns a reference to member tag
 * @return Reference to member tag
 */
int32_t& MPIMessage::Message::tag()
{
    return m_tag;
}


size_t MPIMessage::Message::getKeyMaxCdrSerializedSize(size_t current_alignment)
{
    size_t current_align = current_alignment;




     current_align += 4 + eprosima::fastcdr::Cdr::alignment(current_align, 4);

     


    return current_align;
}

bool MPIMessage::Message::isKeyDefined()
{
   return true;
}

void MPIMessage::Message::serializeKey(eprosima::fastcdr::Cdr &scdr) const
{
    (void) scdr;
     
     
     scdr << m_recipientRank;  
     
}

