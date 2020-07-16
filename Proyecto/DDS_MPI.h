#ifndef __DDS_MPI_H__
#define __DDS_MPI_H__

#include "MPIMessagePubSubTypes.h"
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastrtps/rtps/common/InstanceHandle.h>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.h>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>
#include <vector>

#define DDSMPI_BROADCAST -1
#define DDSMPI_ANY_SOURCE -1
#define DDSMPI_ANY_TAG -1

#define DDSMPI_SUCCESS                      0
#define DDSMPI_INIT_FALSE                   1
#define DDSMPI_ERR_COUNT                    2
#define DDSMPI_ERR_TAG                      4
#define DDSMPI_ERR_RANK                     6
#define DDSMPI_INIT_ERROR                   8
#define DDSMPI_ERR_WAIT                     10
#define DDSMPI_ERR_TAKE                     12
#define DDSMPI_ERR                          14


using namespace eprosima::fastdds::rtps;
using namespace eprosima::fastdds::dds;

// ESTRUCTURAS : Request (de MPI_Request) y Status (de MPI_Status)

typedef struct status_t Status;
typedef struct request_t *Request;

//Se utiliza para guardar información sobre operaciones de recepción de mensajes
struct status_t{
    int SOURCE;         // Rango del proceso origen 
    int TAG;            // Valor de la etiqueta que tenía el mensaje
    int ERROR;          // Tipo de error
    int cancelled;       
    size_t ucount;      // Tamaño del mensaje en bytes
};
// Se utiliza para identificar operaciones no bloqueantes, guarda una etiqueta identificativa para ellas
struct request_t{
    //COMPLETAR
};



class DDS_MPI {
public:
    // FUNCIONES PUBLICAS

    // Constructor
    DDS_MPI()
        : participant(nullptr)
        //, publisher(nullptr)
        , subscriber(nullptr)
        , topic(nullptr)
        , type (new MPIMessage::MessagePubSubType()) // Comprobar si es necesario que sea así
        , rank(0)
        , size(0)
        , initialized(false)
    {}
    // Destructor
    ~DDS_MPI(){}

    // Constructor de copia. Prohibido
    DDS_MPI(const DDS_MPI& obj) = delete;

    // Constructor de movimiento. Prohibido
    DDS_MPI(DDS_MPI&& obj) = delete;

    // Operador de asignación de copia. Prohibido
    DDS_MPI& operator=(const DDS_MPI& obj) = delete;

    // Operador de asignaciones de movimiento. Prohibido
    DDS_MPI& operator=(DDS_MPI&& obj) = delete;
    
    /**Aquí lo equivalente a MPI_Init, la creación del participante, 
     *creación del publicador y suscriptor, creación del topic, ... 
     */
    int Initialize(int &argc, char* argv[]); 

    void Finalize();

    int Send(void *buf, int count, int dest, int tag);

    int ISend(void *buf, int count, int dest, int tag, Request *request);
    //void Test(Request *request, int *flag, Status *status);

    //void Testsome (int incount, Request array_of_request[], int *outcount,
    //int array_of_indices[], Status array_of_statuses[])

    int Recv(void *buf, int count, int source, int tag, Status *status);

    int IRecv(void *buf, int count, int source, int tag, Request *request);

    int GetRank() const { return rank; }

    int GetSize() const { return size;}

    //Me queda: MPI_Allgather, MPI_Allgatherv, MPI_Comm_group, MPI_Group_incl, MPI_Comm_create, MPI_Group_free
private:
    // ATRIBUTOS
    MPIMessage::Message msg;

    DomainParticipantFactory *participantFactory;

    DomainParticipant* participant;

    Publisher* publisher;

    DataWriter* writer;

    Subscriber* subscriber;

    DataReader* reader;

    Topic* topic;

    TypeSupport type;

    std::vector<eprosima::fastrtps::rtps::InstanceHandle_t> instances;

    std::list<MPIMessage::Message> messages;


    int rank;
    int size;

    bool initialized; // Estado de la clase. Indica si se ha inicializado o no.

    int group; // Grupo de pertenencia

    // LISTENERS
    // Listener del publicador
    class PubListener : public DataWriterListener
    {
        public:
            PubListener()
            : matched(0)
        {
        }

        ~PubListener() override
        {
        }

        void on_publication_matched( 
            DataWriter* writer,
            const PublicationMatchedStatus& info)
        {
            if (info.current_count_change == 1)
            {
                matched = info.total_count;
                std::cout << "Publisher matched." << std::endl;
            }
            else if (info.current_count_change == -1)
            {
                matched = info.total_count;
                std::cout << "Publisher unmatched." << std::endl;
            }
            else
            {
                std::cout << info.current_count_change
                        << " is not a valid value for PublicationMatchedStatus current count change." << std::endl;
            }
        }


        std::atomic_int matched;
    } pubListener;

    // Listener del suscriptor
    class SubListener : public DataReaderListener
    {
    public:

        SubListener()
            : matched(0)
        {
        }

        ~SubListener() override
        {
        }

        void on_subscription_matched(
                DataReader*,
                const SubscriptionMatchedStatus& info) override
        {
            if (info.current_count_change == 1)
            {
                matched = info.total_count;
                std::cout << "Subscriber matched." << std::endl;
            }
            else if (info.current_count_change == -1)
            {
                matched = info.total_count;
                std::cout << "Subscriber unmatched." << std::endl;
            }
            else
            {
                std::cout << info.current_count_change
                        << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
            }
        }

        void on_data_available(
                DataReader* reader) override
        {
            
        }

        std::atomic_int matched;

    } subListener;

    // FUNCIONES PRIVADAS (auxiliares)

    bool GetArg(int &argc, char* argv[], const char *arg, int &value);
    
    //bool CreatePublisher()

    //bool CreateSubscriber()


};

#endif
