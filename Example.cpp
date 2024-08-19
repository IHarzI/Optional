#include <iostream>
#include <string>
#include <mutex>
#include <thread>

struct ErrorGame
{
	const char* Name = "ErrorGame";
	int Code = 999;
};

struct Success
{
	const char* Name = "Success";
	int OpSuccess = 150;
};

harz::OptionalPair<Success, ErrorGame> GetResultGame()
{
	harz::OptionalPair<Success, ErrorGame> Result{};
	if (rand() % 25 > 19)
	{
		Result.SetValueA(Success{});
	}
	else
		Result.SetValueB(ErrorGame{});

	return std::move(Result);
}

#define NameGet(pair) std::string{pair.IsTypeA() ? pair.GetValueA().Name : pair.GetValueB().Name}


int main()
{
  harz::Optional<int> intOpt{};
	intOpt.SetValue(5);
	intOpt.SetValue(15);
	auto newOpt = intOpt;
	harz::OptionalPair<int, bool> PairOpt;
	PairOpt.SetValueA(25325);
	auto IntPairOpt = PairOpt.GetValueA();
	PairOpt.SetValueB(false);
	auto boolPairOpt = PairOpt.GetValueB();
	auto MovedPairOpt = std::move(PairOpt);
	auto movedOpt = std::move(intOpt);
	harz::Optional<std::mutex> MutexOpt{};
	MutexOpt.DefaultInitialize();

	harz::Optional<int> StatisticsInt{};
	StatisticsInt.DefaultInitialize();
  
	auto lambda = [&]() {
		std::lock_guard lg{ MutexOpt.GetValue()}; 
		int Statistics = StatisticsInt.GetValue();
		Statistics += rand() % 3;
		StatisticsInt.SetValue(Statistics);
	};
	auto ResultMsgOpt = GetResultGame();
	auto ResultMsgOptSome2 = GetResultGame();
	auto ResultMsgOptAR53 = GetResultGame();
	for (int i = 0; i < 1000; i++)
	{
		auto result = GetResultGame();
		std::cout << " || New Pair: " << NameGet(result) << " || ";
	}
	StatisticsInt.GetValue()++;
	std::cout << " \n Statistics Begin: " << StatisticsInt.GetValue() << '\n';
	std::thread Tr1(lambda);
	std::thread Tr2(lambda);
	std::thread Tr3(lambda);
	Tr1.join();
	Tr2.join();
	Tr3.join();
	std::cout << " \n Statistics End: " << StatisticsInt.GetValue() << '\n';

	return 0;
};
