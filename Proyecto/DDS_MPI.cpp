#include "DDS_MPI.h"
#include <iostream>
#include <stdlib.h>
#include <string>
#include <algorithm>    // std::copy

#include <fastrtps/types/TypesBase.h>
#include <fastdds/dds/subscriber/SampleInfo.hpp>


//---------- FUNCIONES PUBLICAS ----------
int DDS_MPI::Initialize(int &argc, char* argv[])
{
    //std::cout << "Se ha llamado a Initialize" << std::endl;

    //1. Obtenemos rank y size
    if(!GetArg(argc, argv, "-r", rank)){
        rank = 0;
    }
    if(!GetArg(argc, argv, "-s", size)){
        size = 1;
    }
    if(rank >= size){ // Error
        std::cerr << "Error in Initialize(): rank must be less than size" << std::endl;
        return DDSMPI_ERR_RANK;
    }
    //2. Creamos la fabrica de participantes. Se utiliza para crear o eliminar participantes.
    // DomainParticipantFactory es un singleton que se accede a el lanzando la función estática get_instance()
    // Dejamos las QoS de DomainParticipantFactory por defecto. Solo hay una QoS, autoenable_created_entities, que por defecto está a true
    participantFactory = DomainParticipantFactory::get_instance();

    //3. Creamos un participante de dominio
    // domain-id = 0, Qos modificadas, listener = nullptr, Status mask = all
    DomainParticipantQos participant_qos = participantFactory -> get_default_participant_qos();
    
    // ----Cofiguración de las QoS----
    // UserDataQosPolicy, EntityFactoryQosPolicy : Por defecto
    // ParticipantResourceLimitsQos
    // -RemoteLocatorsAllocationAttributes : Por defecto
    // -ResourceLimitedContainerConfig : Por defecto
    // -SendBuffersAllocationAttributes : Por defecto
    // -VariableLengthDataLimits : Por defecto
    // PropertyPolicyQos : Por defecto
    // WireProtocolConfigQos : Por defecto
    // TransportConfigQos
    int transport = 2; // Por defecto, UDPv4 normal
    GetArg(argc,argv,"-t", transport);
    if(transport == 0){ // UDPv4 modificada
        participant_qos.transport().use_builtin_transports = false;
        std::shared_ptr<UDPv4TransportDescriptor> udp_transport = std::make_shared<UDPv4TransportDescriptor>();
        udp_transport -> sendBufferSize = 9216;
        udp_transport -> receiveBufferSize = 9216;
        udp_transport -> non_blocking_send = true;
        participant_qos.transport().user_transports.push_back(udp_transport);
        std::cout << "Tipo de transporte: UDPv4 modificado" << std::endl; 
    }else if(transport == 1){ // SHM
        participant_qos.transport().use_builtin_transports = false;
        std::shared_ptr<SharedMemTransportDescriptor> shm_transport = std::make_shared<SharedMemTransportDescriptor>();
        participant_qos.transport().user_transports.push_back(shm_transport);
        std::cout << "Tipo de transporte: SHM" << std::endl; 
    }else{ // UDPv4 normal
        std::cout << "Tipo de transporte: UDPv4 por defecto" << std::endl; 
    }

    participant = participantFactory -> create_participant(0, participant_qos);
    if(participant == nullptr){
        std::cerr << "Error in Initialize(): create_participant" << std::endl;
        return DDSMPI_INIT_ERROR;
    }

    //4. Registramos el tipo 
    if(type.register_type(participant, type.get_type_name()) != eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK){
        std::cerr << "Error in Initialize(): register_type" << std::endl;
        return DDSMPI_INIT_ERROR;
    }

    //5. Creamos el topic
    // Lo creamos con Qos por defecto, listener = nullptr, Status mask = all
    std::string type_name = type.get_type_name();
    //std::cout << type_name << std::endl;
    topic = participant -> create_topic("MPITopic", type_name, TOPIC_QOS_DEFAULT);
    if(topic == nullptr){
        std::cerr << "Error in Initialize(): create_topic" << std::endl;
        return DDSMPI_INIT_ERROR;
    }

    //6. Creamos el publisher y data writer
    //typedef std::vector<std::string> StringSeq;
    //StringSeq part;
    PublisherQos pub_qos;
    // ----Cofiguración de las QoS del publisher----
    // Por defecto
    participant -> get_default_publisher_qos(pub_qos);
    publisher = participant -> create_publisher(pub_qos);
    if(publisher == nullptr){
        std::cerr << "Error in Initialize(): create_publisher" << std::endl;
        return DDSMPI_INIT_ERROR;
    }
    
    ReliabilityQosPolicy reliability;
    // ----Configuración de las QoS del DataWriter----
    DurabilityQosPolicy durability;
    durability.kind = VOLATILE_DURABILITY_QOS;
    
    reliability.kind = BEST_EFFORT_RELIABILITY_QOS;
    reliability.max_blocking_time={0,100000}; //{second,nsecond} 100ms
    
    PublishModeQosPolicy publish_mode;
    publish_mode.kind = ASYNCHRONOUS_PUBLISH_MODE;

    DataWriterQos dw_qos;
    publisher -> get_default_datawriter_qos(dw_qos);
    dw_qos.reliability(reliability);
    dw_qos.durability(durability);
    dw_qos.publish_mode(publish_mode);

    writer = publisher -> create_datawriter(topic, dw_qos, &pubListener);
    if(writer == nullptr){
        std::cerr << "Error in Initialize(): create_datawriter" << std::endl;
        return DDSMPI_INIT_ERROR;
    }

    //7. Creamos un "Instance Handle" para cada recipiente
    MPIMessage::Message sample;
    for(int i = 0; i<=size; i++){
        sample.recipientRank((i==size) ? DDSMPI_BROADCAST : i);
        instances.push_back(writer->register_instance(&sample));
    }

    //8. Creamos el subscriber y dataReader 
    SubscriberQos sub_qos;
    // ----Cofiguración de las QoS del publisher----
    // Por defecto
    participant ->get_default_subscriber_qos(sub_qos);
    subscriber = participant -> create_subscriber(sub_qos);
    if(subscriber == nullptr){
        std::cerr << "Error in Initialize(): create_subscriber" << std::endl;
        return DDSMPI_INIT_ERROR;
    }

    DataReaderQos dr_qos;
    // ----Configuración de las QoS del DataWriter----
    subscriber -> get_default_datareader_qos(dr_qos);
    dr_qos.reliability(reliability);
    dr_qos.durability(durability);

    reader = subscriber -> create_datareader(topic, dr_qos, &subListener);
    if(reader == nullptr){
        std::cerr << "Error in Initialize(): create_datareader" << std::endl;
        return DDSMPI_INIT_ERROR;
    }

    //9. Esperamos por las asociaciones entre el dataReader y dataWriter
    // Wait del publicador
    /*
    std::unique_lock<std::mutex> lock(pubListener.mutex);
    pubListener.cv.wait(lock, [&pubListener] 
    {
        return pubListener.matched;
    });
    */
    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if(pubListener.matched==size){
            //std::cout <<"PRUEBA: Wait publisher"<<std::endl;
            break;
        }
    }
    // Wait del suscriptor 
     while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if(subListener.matched==size){
            //std::cout <<"PRUEBA: Wait subscriber"<<std::endl;
            break;
        }
    }

    std::cout << "Fin Initialize()" << std::endl;

    initialized = true;
    return DDSMPI_SUCCESS;
}

int DDS_MPI::Finalize()
{
    // Chequeo de errorres
    if(!initialized){
        std::cerr << "Finalize(): initialized is FALSE. Initialize first" << std::endl;
        return DDSMPI_INIT_FALSE;
    }

    // Eliminamos el dataReader
    if(subscriber -> delete_datareader(reader) != eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK)
    {
        std::cerr << "Finalize(): delete_datareader" << std::endl;
    }
    reader = nullptr;

    // Esperamos a que el resto de procesos borren su dataReader
    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if(pubListener.matched==0){
            std::cout <<"PRUEBA 3"<<std::endl;
            break;
        }
    }

    // Eliminamos todas las instancias
    MPIMessage::Message sample;
    for (int i=0; i<=size; i++) {
        sample.recipientRank((i == size) ? DDSMPI_BROADCAST : i); 
        writer -> unregister_instance(&sample, instances[i]);
    }
    instances.clear();

    // Eliminamos el participant
    if(participant != nullptr){
        participantFactory -> delete_participant(participant);
        participant = nullptr;
    }

}


int DDS_MPI::Send(void *buf, int count, int dest, int tag)
{
    // Chequeo de errorres
    if(!initialized){
        std::cerr << "Send(): initialized is FALSE. Initialize first" << std::endl;
        return DDSMPI_INIT_FALSE;
    }
    if(count < 0){
        std::cerr << "Send(): count must be a non-negative value" << std::endl;
        return DDSMPI_ERR_COUNT;
    }
    if(tag < 0){
        std::cerr << "Send(): tag must be a non-negative value" << std::endl;
        return DDSMPI_ERR_TAG;
    }
    if(dest < 0 || dest >= size){
        std::cerr << "Send(): dest rank not valid" << std::endl;
        return DDSMPI_ERR_RANK;
    }

    MPIMessage::Message sample;
    sample.senderRank(rank);
    sample.recipientRank(dest);
    sample.tag(tag);
    sample.buffer().resize(count);
    //memcpy(static_cast<void*>(sample.buffer().data()),buf,count);
    //std::copy(buf,static_cast<void*> (buf + count),sample.buffer().begin());
    uint8_t *aux = (uint8_t*) buf;

    std::copy(aux,aux+count,sample.buffer().begin());
/*
    for(int i=0; i<count; i++){
        sample.buffer().begin()[i]=aux[i];
    }
*/

    //ReturnCode_t rt = writer->write(&sample,instances[(dest==DDSMPI_BROADCAST) ? size : dest]);
    // La identidad de la instancia debe deducirse automaticamente de la clave de los datos de la instancia
    ReturnCode_t rt = writer->write((void *)&sample,eprosima::fastrtps::rtps::InstanceHandle_t()); 
    if(rt == eprosima::fastrtps::types::ReturnCode_t::RETCODE_PRECONDITION_NOT_MET){
        std::cerr << "Error in Send(): write() - RETCODE_PRECONDITION_NOT_MET" << std::endl;
        return DDSMPI_ERR;
    }else if(rt == eprosima::fastrtps::types::ReturnCode_t::RETCODE_TIMEOUT){
        std::cerr << "Error in Send(): write() - RETCODE_TIMEOUT" << std::endl;
        return DDSMPI_ERR;
    }else if(rt == eprosima::fastrtps::types::ReturnCode_t::RETCODE_ERROR){
        std::cerr << "Error in Send(): write()" << std::endl;
        return DDSMPI_ERR;
    }
    return DDSMPI_SUCCESS;
}

int DDS_MPI::Recv(void *buf, int count, int source, int tag, Status *status)
{
    // Chequeo de errores
    if(!initialized){
        std::cerr << "Recv(): initialized is FALSE. Initialize first" << std::endl;
        return DDSMPI_INIT_FALSE;
    }
    if(count < 0){
        std::cerr << "Recv(): count must be a non-negative value" << std::endl;
        return DDSMPI_ERR_COUNT;
    }
    if(tag < 0){
        std::cerr << "Recv(): tag must be a non-negative value" << std::endl;
        return DDSMPI_ERR_TAG;
    }
    if(source < 0 || source >= size){
        std::cerr << "Recv(): source rank not valid" << std::endl;
        return DDSMPI_ERR_RANK;
    }

    uint8_t *aux = (uint8_t*) buf; // Puntero auxiliar para modificar los datos de buff (salida)

    // Primero, comprobamos si algun mensaje pendiente verifica los criterios
    // En ese caso, lo devolvemos con aux y eliminamos el mensaje pendiente  
    for (std::list<MPIMessage::Message>::iterator it = messages.begin(); it != messages.end(); it++) {
        if ( ((source == DDSMPI_ANY_SOURCE) || (source == it->senderRank())) && 
             ((tag == DDSMPI_ANY_TAG) || (tag == it->tag()))) {
                 status->SOURCE = it->senderRank();
                 status->TAG = it->tag();
                 for (int i = 0; i < count; i++)
                 {
                   aux[i] = it->buffer()[i];
                 }
                 messages.erase(it);
                 return DDSMPI_SUCCESS;
        }
    }

    Duration_t infinite = eprosima::fastrtps::c_TimeInfinite;
    MPIMessage::Message msg;
    SampleInfo info;
    bool newMessage;
    // Recv es bloqueante
    while(true){
        // Esperamos a un mensaje no leido un tiempo infinito (bloqueante)
        newMessage = reader->wait_for_unread_message(infinite);
        if(!newMessage){
            std::cerr << "Error Recv(): wait_for_unread_message" << std::endl;
            return DDSMPI_ERR_WAIT;
        }
        // Recibimos el mensaje. take elimina el mensaje una vez ha sido leido.
        if(eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK != reader->take_next_sample(&msg,&info)){
            std::cerr << "Error Recv(): take_next_sample" << std::endl;
            return DDSMPI_ERR_TAKE;
        }
        if(!info.valid_data){
            continue;   // Si se recibe un mensaje incorrecto, intentamos de nuevo
        }

        // Comprobamos si se verifican los criterios, y en tal caso aceptamos el mensaje
        if ( ((source == DDSMPI_ANY_SOURCE) || (source == msg.senderRank())) && 
             ((tag == DDSMPI_ANY_TAG) || (tag == msg.tag()))) {
                 status -> SOURCE = msg.senderRank();
                 status -> TAG = msg.tag();
                 for (int i = 0; i < count; i++)
                    aux[i] = msg.buffer()[i];
                 return DDSMPI_SUCCESS;
        }

        // No se verifican los criterios, por lo que lo encolamos y continuamos
        messages.push_back(msg);
    
    }

}


int DDS_MPI::ISend(void *buf, int count, int dest, int tag, Request *request)
{
    // HACER
}

int DDS_MPI::IRecv(void *buf, int count, int source, int tag, Request *request)
{
    // TENGO QUE TERMINARLO

    // Chequeo de errores
    if(!initialized){
        std::cerr << "IRecv(): initialized is FALSE. Initialize first" << std::endl;
        return DDSMPI_INIT_FALSE;
    }
    if(count < 0){
        std::cerr << "IRecv(): count must be a non-negative value" << std::endl;
        return DDSMPI_ERR_COUNT;
    }
    if(tag < 0){
        std::cerr << "IRecv(): tag must be a non-negative value" << std::endl;
        return DDSMPI_ERR_TAG;
    }
    if(source < 0 || source >= size){
        std::cerr << "IRecv(): source rank not valid" << std::endl;
        return DDSMPI_ERR_RANK;
    }

    // Es una función no bloqueante, ponemos un tiempo de bloqueo de 100ms
    Duration_t time = {0,100000};
    MPIMessage::Message msg;
    SampleInfo info;

    bool newMessage = reader->wait_for_unread_message(time);
    if(!newMessage){
        // No se ha recibido ningún mensaje (timeout alcanzado)
        // Debemos continuar pues es no bloqueante
        return 99; // Cambiar esto
    }else{
        // Se ha recibido un mensaje
        // Recibimos el mensaje. take elimina el mensaje una vez ha sido leido.
        if(eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK != reader->take_next_sample(&msg,&info)){
            std::cerr << "Error IRecv(): take_next_sample" << std::endl;
            return DDSMPI_ERR_TAKE;
        }   
        if(!info.valid_data){ // Mensaje incorrecto
            return 99;   // Cambiar esto
        }
        //----------TERMINAR------------
        
    }       
}


//---------- FUNCIONES PRIVADAS ----------

// Cuidado: Los argumentos con sus valores deben estar separados con un espacio
bool DDS_MPI::GetArg(int &argc, char* argv[], const char *arg, int &value)
{
    bool found = false;
    for(int i = 0; i < argc; i++){
        if(strcmp(argv[i],arg) == 0){
            found = true;
            value = atoi(argv[i+1]);
        }
    }
    return found;
}