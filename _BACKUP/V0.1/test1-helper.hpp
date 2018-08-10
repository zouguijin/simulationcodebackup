
// test1-helper.hpp

// fengzhuang guanyu measurement de API

#include "table/measurements-accessor.hpp"
#include "face/face.hpp"
#include "table/fib.hpp"
#include "table/pit.hpp"
#include <map>

using namespace std;

namespace nfd {

	namespace test1helper {

		class Test1Helper {
			public:
				Test1Helper();

				Test1Helper&
				getMeasurementAndFace();

			private:

				measurements::MeasurementsAccessor mmt_accessor;

		}
	}
}